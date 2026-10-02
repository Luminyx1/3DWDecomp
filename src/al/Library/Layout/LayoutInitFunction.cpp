#include "Library/Layout/LayoutInitFunction.hpp"

#include <common/aglDrawContext.h>
#include <eui/euiConstantBuffer.h>
#include <eui/euiDrawInfoEx.h>
#include <eui/euiFontMgr.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiMultiArcResourceAccessor.h>
#include <eui/euiNwAllocator.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiTextSearcher.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <nn/ui2d/ui2d_DrawInfo.h>
#include <nn/ui2d/ui2d_Init.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Parts.h>
#include <nn/ui2d/ui2d_TextBox.h>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/Layout/EuiScreen.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Library/Layout/LayoutResource.hpp"
#include "Library/Layout/LayoutSystem.hpp"
#include "Library/LiveActor/HitReactionKeeper.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Message/CustomTagProcessor.hpp"
#include "Library/Message/LanguageUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Controller/PadRumbleKeeper.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"

namespace al {
namespace {
/**
 * Returns the root pane of a layout.
 * @param pLayout layout user
 * @return the root pane
 */
inline nn::ui2d::Pane* getRootPane(const IUseLayout* pLayout) {
    return pLayout->getLayoutKeeper()->getLayout()->GetRootPane();
}

/**
 * Finds the string buffer size of a text pane in a text box buffer list.
 * @param pSize output buffer size
 * @param pIter text box buffer list
 * @param pPaneName text pane name
 * @param pPartsName name of the parts layout holding the pane, or nullptr
 * @return whether an entry with a size was found
 */
bool tryFindTextBoxBufferSize(s32* pSize, const ByamlIter* pIter, const char* pPaneName,
                              const char* pPartsName) {
    for (s32 i = 0; i < pIter->getSize(); i++) {
        ByamlIter entry;

        if (!pIter->tryGetIterByIndex(&entry, i)) {
            continue;
        }

        const char* paneName = nullptr;

        if (!entry.tryGetStringByKey(&paneName, "PaneName")) {
            continue;
        }

        if (!isEqualString(paneName, pPaneName)) {
            continue;
        }

        const char* partsName = nullptr;
        entry.tryGetStringByKey(&partsName, "PartsName");

        if (partsName == nullptr || (pPartsName != nullptr && isEqualString(partsName, pPartsName))) {
            s32 size;

            if (!entry.tryGetIntByKey(&size, "Size")) {
                return false;
            }

            *pSize = size;
            return true;
        }
    }

    return false;
}

/**
 * Initializes the text boxes of a pane tree with their messages.
 * @param pPane root of the pane tree
 * @param pMessageSystem message system
 * @param pLayout layout
 * @param pPartsLayout layout of the parts holding the panes
 * @param pIter text box buffer list, or nullptr
 */
void initPaneMessageRecursive(nn::ui2d::Pane* pPane, const MessageSystem* pMessageSystem,
                              const nn::ui2d::Layout* pLayout,
                              const nn::ui2d::Layout* pPartsLayout, const ByamlIter* pIter) {
    for (nn::util::IntrusiveListNode* node = pPane->m_Children.GetNext();
         node != &pPane->m_Children; node = node->GetNext()) {
        auto* parts = eui::DynamicCast<nn::ui2d::Parts>(pPane);

        if (parts != nullptr) {
            pPartsLayout = parts->m_pLayout;
        }

        nn::ui2d::Pane* child = nn::ui2d::Pane::FromLink(node);
        isMatchString(child->GetName(), MatchStr("Txt*"));

        if (*reinterpret_cast<void* const*>(reinterpret_cast<const u8*>(child) + 0xe0) ==
            nullptr) {
            initPaneMessageRecursive(child, pMessageSystem, pLayout, pPartsLayout, pIter);
            continue;
        }

        s32 bufferSize = 0x10;

        if (pIter != nullptr) {
            tryFindTextBoxBufferSize(&bufferSize, pIter, child->GetName(), pPartsLayout->GetName());
        }

        initTextBoxRecursiveWithSelfTextId(child, bufferSize, pMessageSystem, pLayout,
                                           pPartsLayout);
    }
}

/**
 * Reallocates the string buffers of the text boxes of a pane tree listed in a buffer list.
 * @param pPane root of the pane tree
 * @param pIter text box buffer list
 */
void reallocateTextBoxStringBufferRecursive(nn::ui2d::Pane* pPane, const ByamlIter* pIter) {
    auto* textBox = eui::DynamicCast<nn::ui2d::TextBox>(pPane);

    if (textBox != nullptr) {
        s32 bufferSize;

        if (tryFindTextBoxBufferSize(&bufferSize, pIter, textBox->GetName(), nullptr)) {
            reallocateTextBoxStringBuffer(textBox, bufferSize);
        }
    }

    for (nn::util::IntrusiveListNode* node = pPane->m_Children.GetNext();
         node != &pPane->m_Children; node = node->GetNext()) {
        reallocateTextBoxStringBufferRecursive(nn::ui2d::Pane::FromLink(node), pIter);
    }
}
}  // namespace

/**
 * Makes the layout allocator use the current heap.
 */
LayoutAllocatorInScope::LayoutAllocatorInScope() {
    eui::NwAllocator::initialize(getCurrentHeap());
}

/**
 * Resets the layout allocator.
 */
LayoutAllocatorInScope::~LayoutAllocatorInScope() {
    nn::ui2d::Initialize(nullptr, nullptr, nullptr);
}

/**
 * Makes the path of a layout archive, preferring an existing localized archive.
 * @param pOut output path
 * @param rName layout archive name
 * @param isLocalized whether to look for a localized archive
 * @return whether a localized archive was found
 */
bool makeLayoutArchivePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                           bool isLocalized) {
    if (!isLocalized) {
        pOut->format("LayoutData/%s", rName.cstr());
        return false;
    }

    if (isEqualString(rName.cstr(), "TrialRating")) {
        StringTmp<256> path("LayoutData/%s", rName.cstr());
        makeLocalizedArchivePathByCountryCode(pOut, path);
    } else {
        StringTmp<256> path("LayoutData/%s", rName.cstr());
        makeLocalizedArchivePath(pOut, path);
    }

    if (isExistArchive(*pOut)) {
        return true;
    }

    pOut->format("LayoutData/%s", rName.cstr());
    return false;
}

/**
 * Makes the path of a layout resource.
 * @param pOut output path
 * @param rName resource name
 * @param isLocalized whether the resource is in the current language's folder
 */
void makeLayoutResourcePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                            bool isLocalized) {
    if (isLocalized) {
        pOut->format("%s/%s", getLanguageString(), rName.cstr());
        return;
    }

    pOut->format("Common/%s", rName.cstr());
}

/**
 * Gives a layout actor a copy of the scene info.
 * @param pActor layout actor
 * @param rInfo layout init info
 */
void initLayoutSceneInfo(LayoutActor* pActor, const LayoutInitInfo& rInfo) {
    LayoutSceneInfo* sceneInfo = new LayoutSceneInfo();
    *sceneInfo = rInfo;
    pActor->initSceneInfo(sceneInfo);
}

/**
 * Creates the effect keeper of a layout actor, following its root pane.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pName effect keeper name
 */
void initLayoutEffectKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName) {
    pActor->initEffectKeeper(
        new EffectKeeper(rInfo.getEffectSystemInfo(), pName, nullptr, nullptr,
                         reinterpret_cast<const sead::Matrix34f*>(getRootPane(pActor)->GetGlobalMtx())));
}

/**
 * Initializes the sound effect keeper of a layout actor.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pName sound effect user name
 */
void initLayoutSeKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName) {
    const AudioDirector* audioDirector = rInfo.getAudioDirector();
    AudioKeeper* audioKeeper;

    if (pActor->getAudioKeeper() != nullptr) {
        audioKeeper = pActor->getAudioKeeper();
    } else {
        audioKeeper = alAudioKeeperFunction::createAndInitAudioKeeper(audioDirector, false);
        pActor->initAudioKeeper(audioKeeper);
    }

    audioKeeper->initSeKeeper(rInfo.getAudioDirector(), pName, nullptr, nullptr, nullptr, "サブ");
}

/**
 * Initializes the music keeper of a layout actor.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pName music user name
 */
void initLayoutBgmKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName) {
    const AudioDirector* audioDirector = rInfo.getAudioDirector();
    AudioKeeper* audioKeeper;

    if (pActor->getAudioKeeper() != nullptr) {
        audioKeeper = pActor->getAudioKeeper();
    } else {
        audioKeeper = alAudioKeeperFunction::createAndInitAudioKeeper(audioDirector, false);
        pActor->initAudioKeeper(audioKeeper);
    }

    audioKeeper->initBgmKeeper(rInfo.getAudioDirector(), pName);
}

/**
 * Creates the hit reaction keeper of a layout actor if it has hit reactions.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pResource layout resource
 * @param pName hit reaction file suffix
 */
void initHitReactionKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                           const Resource* pResource, const char* pName) {
    HitReactionKeeper* hitReactionKeeper = HitReactionKeeper::tryCreate(pActor, pResource, pName);

    if (hitReactionKeeper == nullptr) {
        return;
    }

    hitReactionKeeper->setPadRumbleKeeper(new PadRumbleKeeper(getMainControllerPort()));
    pActor->initHitReactionKeeper(hitReactionKeeper);
}

/**
 * Initializes the text boxes of a layout with their messages.
 * @param pKeeper layout keeper
 * @param rInfo layout init info
 * @param pResource layout resource
 */
void initLayoutMessage(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                       const Resource* pResource) {
    const u8* bufferInfo = tryGetByml(pResource, "InitTextBoxBuffer");
    const LayoutSceneInfo& rSceneInfo = rInfo;

    if (rSceneInfo.getMessageSystem() == nullptr) {
        return;
    }

    auto* layout = static_cast<eui::LayoutEx*>(pKeeper->getLayout());

    if (bufferInfo != nullptr) {
        ByamlIter iter(bufferInfo);
        initPaneMessageRecursive(layout->GetRootPane(), rSceneInfo.getMessageSystem(), layout,
                                 layout, &iter);
    } else {
        initPaneMessageRecursive(layout->GetRootPane(), rSceneInfo.getMessageSystem(), layout,
                                 layout, nullptr);
    }

    eui::IteratePaneForSetupPaneAfterBuild(layout->GetRootPane(), layout);
}

/**
 * Initializes a draw info for the whole layout display.
 * @param pDrawInfo draw info
 * @param pResource graphics resource
 */
void initDrawInfoDefault(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource) {
    f32 left = getLayoutDisplayWidth() * -0.5f;
    f32 top = getLayoutDisplayHeight() * 0.5f;
    f32 right = getLayoutDisplayWidth() * 0.5f;
    f32 bottom = getLayoutDisplayHeight() * -0.5f;
    nn::font::Rectangle rect = {left, top, right, bottom};
    initDrawInfo(pDrawInfo, pResource, rect);
}

/**
 * Initializes a draw info with a perspective projection showing a rectangle.
 * @param pDrawInfo draw info
 * @param pResource graphics resource
 * @param rRect visible rectangle
 */
void initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource,
                  const nn::font::Rectangle& rRect) {
    pDrawInfo->SetGraphicsResource(pResource);

    {
        f32 width = rRect.GetWidth();
        f32 height = rRect.GetHeight();
        f32 halfWidth = (width > 0.0f ? width : -width) * 0.5f;
        f32 halfHeight = (height > 0.0f ? height : -height) * 0.5f;
        f32 distance = halfHeight / 0.1316525f;
        sead::PerspectiveProjection projection(1.0f, 10000.0f, 0.2617994f, halfWidth / halfHeight);
        sead::LookAtCamera camera(sead::Vector3f(0.0f, 0.0f, distance), sead::Vector3f::zero,
                                  sead::Vector3f::ey);
        camera.updateViewMatrix();
        sead::Matrix44f projectionMtx;
        projectionMtx.setMul(projection.getDeviceProjectionMatrix(), camera.getMatrix());
        pDrawInfo->SetProjectionMtx(
            *reinterpret_cast<const nn::util::MatrixT4x4fType*>(&projectionMtx));
    }

    nn::util::MatrixT4x3fType viewMtx;
    viewMtx._m.val[0] = float32x4_t{1.0f, 0.0f, 0.0f, 0.0f};
    viewMtx._m.val[1] = float32x4_t{0.0f, 1.0f, 0.0f, 0.0f};
    viewMtx._m.val[2] = float32x4_t{0.0f, 0.0f, 1.0f, 0.0f};
    pDrawInfo->SetViewMtx(viewMtx);
}

/**
 * Builds the layout of a layout keeper from a layout archive.
 * @param pKeeper layout keeper
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pTagProcessor tag processor of the text boxes
 * @param isLocalized whether the layout archives are localized
 */
void initLayoutKeeper(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                      const Resource* pResource, CustomTagProcessor* pTagProcessor,
                      bool isLocalized) {
    const char* name = getResourceName(pResource);
    const LayoutSystem* layoutSystem = rInfo.getLayoutSystem();
    auto* layoutResource = new (4) LayoutResource(pResource, rInfo.getLayoutSystem(), isLocalized);
    EuiScreen* screen = new EuiScreen();
    screen->initializeForMinimum(layoutSystem->getScreenMgr(), name);
    eui::LayoutEx* layout = screen->mLayout;
    pKeeper->initScreen(screen);
    eui::TextSearcher textSearcher(nullptr, pTagProcessor);
    sead::GraphicsNvn::instance()->lockDrawContext();
    auto* device = reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice());
    nn::ui2d::BuildResultInformation result;
    nn::ui2d::Layout::BuildOption option;
    StringTmp<128> fileName("%s.bflyt", name);
    layout->BuildWithName(&result, device, layoutResource, nullptr, &textSearcher, option,
                          fileName.cstr(), false);
    eui::IteratePaneForSetupPaneAfterBuild(screen->mLayout->GetRootPane(), screen->mLayout);
    sead::GraphicsNvn::instance()->unlockDrawContext();
    pKeeper->initLayout(layout, layoutResource);
    pKeeper->initTagProcessor(pTagProcessor);
    pKeeper->initDrawInfo(rInfo.getDrawInfo());
    pKeeper->setDrawContext(rInfo.getDrawContext());
    layoutResource->RegisterTextureViewToDescriptorPool(eui::RegisterSlotForTexture, nullptr);
}

/**
 * Creates the tag processor of a layout.
 * @param rInfo layout init info
 * @param pResource layout archive
 * @return the tag processor
 */
CustomTagProcessor* createTagProcessor(const LayoutInitInfo& rInfo, const Resource* pResource) {
    return new CustomTagProcessor(rInfo.getLayoutSystem()->getScreenMgr()->getMessageMgr(),
                                  rInfo.getLayoutSystem()->getScreenMgr()->getFontMgr(),
                                  static_cast<const LayoutSceneInfo&>(rInfo).getMessageSystem());
}

/**
 * Reallocates the string buffers of the text boxes listed in the layout's init file.
 * @param pActor layout actor
 * @param pResource layout archive
 * @param pSuffix init file suffix
 */
void reallocateTextBoxStringBuffer(LayoutActor* pActor, const Resource* pResource,
                                   const char* pSuffix) {
    ByamlIter iter;

    if (tryGetLayoutActorInitFileIter(&iter, pResource, "InitTextBoxBuffer", pSuffix)) {
        reallocateTextBoxStringBufferRecursive(pActor->getLayoutKeeper()->getLayout()->GetRootPane(),
                                               &iter);
    }
}
}  // namespace al
