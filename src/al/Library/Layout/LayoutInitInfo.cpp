#include "Library/Layout/LayoutInitInfo.hpp"

#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Parts.h>
#include <nn/ui2d/ui2d_TextBox.h>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutResource.hpp"
#include "Library/Layout/LayoutSystem.hpp"
#include "Library/Layout/LayoutTextPaneAnimator.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Message/CustomTagProcessor.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
/**
 * Finds a pane of a layout by name.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the pane
 */
inline nn::ui2d::Pane* findPane(const IUseLayout* pLayout, const char* pPaneName) {
    return pLayout->getLayoutKeeper()->getLayout()->GetRootPane()->FindPaneByName(pPaneName,
                                                                                    true);
}

/**
 * Creates a tag processor and applies the settings of the layout's tag processor init file.
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pSuffix init file suffix
 * @return the tag processor
 */
CustomTagProcessor* createTagProcessorWithInitFile(const LayoutInitInfo& rInfo,
                                                   const Resource* pResource,
                                                   const char* pSuffix) {
    CustomTagProcessor* tagProcessor = createTagProcessor(rInfo, pResource);
    ByamlIter iter;

    if (!tryGetLayoutActorInitFileIter(&iter, pResource, "InitTagProcessor", pSuffix)) {
        return tagProcessor;
    }

    f32 rubyScale;

    if (iter.tryGetFloatByKey(&rubyScale, "RubyScale")) {
        tagProcessor->setRubyScale(rubyScale);
    }

    f32 rubyCharSpace;

    if (iter.tryGetFloatByKey(&rubyCharSpace, "RubyCharSpace")) {
        tagProcessor->setRubyCharSpace(rubyCharSpace);
    }

    f32 rubyBaseLineOffset;

    if (iter.tryGetFloatByKey(&rubyBaseLineOffset, "RubyBaseLineOffset")) {
        tagProcessor->setRubyBaseLineOffset(rubyBaseLineOffset);
    }

    f32 pictFontScale;

    if (iter.tryGetFloatByKey(&pictFontScale, "PictFontScale")) {
        tagProcessor->setPictFontScale(pictFontScale);
    }

    bool isEnableRuby = true;

    if (iter.tryGetBoolByKey(&isEnableRuby, "IsEnableRuby")) {
        tagProcessor->setEnableRuby(isEnableRuby);
    }

    bool isUseDeviceFontColor = false;

    if (iter.tryGetBoolByKey(&isUseDeviceFontColor, "IsUseDeviceFontColor")) {
        tagProcessor->setUseDeviceFontColor(isUseDeviceFontColor);
    }

    return tagProcessor;
}

/**
 * Initializes the sound effect and music keepers listed in the layout's audio init file.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pSuffix init file suffix
 */
void initLayoutAudio(LayoutActor* pActor, const LayoutInitInfo& rInfo, const Resource* pResource,
                     const char* pSuffix) {
    ByamlIter iter;
    const char* seName = nullptr;
    const char* bgmName = nullptr;

    if (tryGetLayoutActorInitFileIter(&iter, pResource, "InitSound", pSuffix)) {
        iter.tryGetStringByKey(&seName, "Name");
        iter.tryGetStringByKey(&bgmName, "Name");
    } else if (tryGetLayoutActorInitFileIter(&iter, pResource, "InitAudio", pSuffix)) {
        iter.tryGetStringByKey(&seName, "SeUserName");
        iter.tryGetStringByKey(&bgmName, "BgmUserName");
    } else {
        return;
    }

    if (seName != nullptr) {
        initLayoutSeKeeper(pActor, rInfo, seName);
    }

    if (bgmName != nullptr) {
        initLayoutBgmKeeper(pActor, rInfo, bgmName);
    }
}

/**
 * Registers a layout actor to the executors listed in its executor init file.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pSuffix init file suffix
 */
void initLayoutExecutor(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                        const Resource* pResource, const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetLayoutActorInitFileIter(&iter, pResource, "InitExecutor", pSuffix)) {
        return;
    }

    const char* updaterName = nullptr;

    if (iter.tryGetStringByKey(&updaterName, "Updater")) {
        registerExecutorLayoutUpdate(pActor, rInfo.getExecuteDirector(), updaterName);
    }

    const char* drawerName = nullptr;

    if (isTypeArrayByKey(iter, "Drawer")) {
        ByamlIter drawerArray;
        getByamlIterByKey(&drawerArray, iter, "Drawer");
        ByamlIter drawer;

        for (s32 i = 0; drawerArray.tryGetIterByIndex(&drawer, i); i++) {
            drawer.tryGetStringByKey(&drawerName, "CategoryName");
            registerExecutorLayoutDraw(pActor, rInfo.getExecuteDirector(), drawerName);
        }
    } else if (iter.tryGetStringByKey(&drawerName, "Drawer")) {
        registerExecutorLayoutDraw(pActor, rInfo.getExecuteDirector(), drawerName);
    }
}

/**
 * Creates the effect keeper named in the layout's effect init file.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pSuffix init file suffix
 */
void initLayoutEffect(LayoutActor* pActor, const LayoutInitInfo& rInfo, const Resource* pResource,
                      const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetLayoutActorInitFileIter(&iter, pResource, "InitEffect", pSuffix)) {
        return;
    }

    const char* effectName = nullptr;

    if (iter.tryGetStringByKey(&effectName, "Name")) {
        initLayoutEffectKeeper(pActor, rInfo, effectName);
    }
}

/**
 * Sets the main pane group named in the layout's main group init file.
 * @param pActor layout actor
 * @param pResource layout archive
 * @param pSuffix init file suffix
 */
void initLayoutMainGroup(LayoutActor* pActor, const Resource* pResource, const char* pSuffix) {
    ByamlIter iter;

    if (!tryGetLayoutActorInitFileIter(&iter, pResource, "InitMainGroup", pSuffix)) {
        return;
    }

    const char* groupName = nullptr;

    if (iter.tryGetStringByKey(&groupName, "Name")) {
        pActor->setMainGroupName(groupName);
    }
}

/**
 * Initializes a layout actor from a layout archive resource.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pResource layout archive
 * @param pSuffix init file suffix
 * @param isLocalized whether the layout archives are localized
 */
void initLayoutActorImpl(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                         const Resource* pResource, const char* pSuffix, bool isLocalized) {
    LayoutAllocatorInScope allocatorScope;
    CustomTagProcessor* tagProcessor = createTagProcessorWithInitFile(rInfo, pResource, pSuffix);
    initLayoutSceneInfo(pActor, rInfo);
    LayoutKeeper* layoutKeeper = new LayoutKeeper();
    initLayoutKeeper(layoutKeeper, rInfo, pResource, tagProcessor, isLocalized);
    pActor->initLayoutKeeper(layoutKeeper);

    initLayoutExecutor(pActor, rInfo, pResource, pSuffix);
    initLayoutEffect(pActor, rInfo, pResource, pSuffix);
    initLayoutAudio(pActor, rInfo, pResource, pSuffix);
    pActor->initActionKeeper();
    initHitReactionKeeper(pActor, rInfo, pResource, pSuffix);

    initLayoutMainGroup(pActor, pResource, pSuffix);
    initLayoutMessage(pActor->getLayoutKeeper(), rInfo, pResource);
    const nn::util::IntrusiveListNode& partsList = pActor->getLayoutKeeper()->getLayout()->_48;

    if (partsList.GetNext() == &partsList) {
        return;
    }

    s32 partsNum = 0;

    for (const nn::util::IntrusiveListNode* node = partsList.GetNext(); node != &partsList;
         node = node->GetNext()) {
        partsNum++;
    }

    pActor->initLayoutPartsActorKeeper(partsNum);
}

/**
 * Initializes a layout actor from a parts pane of its parent's layout.
 * @param pActor parts layout actor
 * @param pParentActor parent layout actor
 * @param rInfo layout init info
 * @param pPaneName parts pane name
 * @param pSuffix init file suffix
 * @param isLocalized whether the layout archives are localized
 */
void initLayoutPartsActorImpl(LayoutActor* pActor, LayoutActor* pParentActor,
                              const LayoutInitInfo& rInfo, const char* pPaneName,
                              const char* pSuffix, bool isLocalized) {
    nn::util::IntrusiveListNode& partsList = pParentActor->getLayoutKeeper()->getLayout()->_48;

    for (nn::util::IntrusiveListNode* node = partsList.GetNext(); node != &partsList;
         node = node->GetNext()) {
        auto* parts = reinterpret_cast<nn::ui2d::Parts*>(reinterpret_cast<u8*>(node) -
                                                         offsetof(nn::ui2d::Parts, m_PartsList));
        nn::ui2d::Layout* partsLayout = parts->m_pLayout;

        if (!isEqualString(pPaneName, parts->GetName())) {
            continue;
        }

        StringTmp<256> archivePath;
        makeLayoutArchivePath(&archivePath, partsLayout->GetName(), isLocalized);
        Resource* resource = findOrCreateResource(archivePath.cstr(), nullptr);
        LayoutAllocatorInScope allocatorScope;
        CustomTagProcessor* tagProcessor = nullptr;

        {
            ByamlIter iter;

            if (tryGetLayoutActorInitFileIter(&iter, resource, "InitTagProcessor", pSuffix)) {
                tagProcessor = createTagProcessorWithInitFile(rInfo, resource, pSuffix);
            }
        }

        LayoutResource* layoutResource =
            new LayoutResource(resource, rInfo.getLayoutSystem(), false);
        LayoutKeeper* layoutKeeper = new LayoutKeeper();
        initLayoutSceneInfo(pActor, rInfo);
        layoutKeeper->initLayout(partsLayout, layoutResource);

        if (tagProcessor != nullptr) {
            layoutKeeper->initTagProcessor(tagProcessor);
        }

        pActor->initLayoutKeeper(layoutKeeper);

        initLayoutEffect(pActor, rInfo, resource, pSuffix);
        initLayoutAudio(pActor, rInfo, resource, pSuffix);
        pActor->initActionKeeper();
        initHitReactionKeeper(pActor, rInfo, resource, pSuffix);

        initLayoutMainGroup(pActor, resource, pSuffix);
        pParentActor->getLayoutKeeper()->getLayout()->GetResourceAccessor()
            ->RegisterTextureViewToDescriptorPool(eui::RegisterSlotForTexture, nullptr);
        ByamlIter iter;
        reallocateTextBoxStringBuffer(pActor, resource, pSuffix);
        registerLayoutPartsActor(pParentActor, pActor);
        return;
    }
}
}  // namespace

/**
 * Creates an empty layout init info.
 */
LayoutInitInfo::LayoutInitInfo() = default;

/**
 * Sets the systems used to initialize layout actors.
 * @param pExecuteDirector execute director
 * @param pEffectSystemInfo effect system info
 * @param pSceneObjHolder scene object holder
 * @param pAudioDirector audio director
 * @param pCameraDirector camera director
 * @param pSceneCameraInfo scene camera info
 * @param pLayoutSystem layout system
 * @param pMessageSystem message system
 * @param pGamePadSystem game pad system
 * @param pPadRumbleDirector pad rumble director
 */
void LayoutInitInfo::init(ExecuteDirector* pExecuteDirector,
                          const EffectSystemInfo* pEffectSystemInfo,
                          SceneObjHolder* pSceneObjHolder, const AudioDirector* pAudioDirector,
                          CameraDirector* pCameraDirector, SceneCameraInfo* pSceneCameraInfo,
                          const LayoutSystem* pLayoutSystem, const MessageSystem* pMessageSystem,
                          const GamePadSystem* pGamePadSystem,
                          PadRumbleDirector* pPadRumbleDirector) {
    mCameraDirector = pCameraDirector;
    mSceneCameraInfo = pSceneCameraInfo;
    mPadRumbleDirector = pPadRumbleDirector;
    mSceneObjHolder = pSceneObjHolder;
    mMessageSystem = pMessageSystem;
    mGamePadSystem = pGamePadSystem;
    mFontMgr = pLayoutSystem->getScreenMgr()->getFontMgr();
    mExecuteDirector = pExecuteDirector;
    mEffectSystemInfo = pEffectSystemInfo;
    mAudioDirector = pAudioDirector;
    mLayoutSystem = pLayoutSystem;
}

/**
 * Returns the message system.
 * @return the message system
 */
const MessageSystem* LayoutInitInfo::getMessageSystem() const {
    return mMessageSystem;
}

/**
 * Initializes a layout actor from a layout archive.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pArchiveName layout archive name
 * @param pSuffix init file suffix
 */
void initLayoutActor(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pArchiveName,
                     const char* pSuffix) {
    StringTmp<256> archivePath("LayoutData/%s", pArchiveName);
    initLayoutActorImpl(pActor, rInfo, findOrCreateResource(archivePath, nullptr), pSuffix, false);
}

/**
 * Initializes a layout actor from a localized layout archive.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pArchiveName layout archive name
 * @param pSuffix init file suffix
 */
void initLayoutActorLocalized(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                              const char* pArchiveName, const char* pSuffix) {
    StringTmp<256> archivePath;
    makeLayoutArchivePath(&archivePath, pArchiveName, true);
    initLayoutActorImpl(pActor, rInfo, findOrCreateResource(archivePath, nullptr), pSuffix, true);
}

/**
 * Does nothing.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param pArchiveName layout archive name
 * @param pMessageArchiveName message archive name
 * @param pSuffix init file suffix
 */
void initLayoutActorUseOtherMessage(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                    const char* pArchiveName, const char* pMessageArchiveName,
                                    const char* pSuffix) {}

/**
 * Creates the text pane animator of a layout actor.
 * @param pActor layout actor
 * @param pPaneName text pane name
 */
void initLayoutTextPaneAnimator(LayoutActor* pActor, const char* pPaneName) {
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(pActor, pPaneName));
    auto* animator = new LayoutTextPaneAnimator(textBox, pActor);
    animator->initEuiLetterAnimCtrl(pActor->getLayoutKeeper()->getScreen()->mLayout);
    pActor->initTextPaneAnimator(animator);
}

/**
 * Creates the text pane animator of a layout actor whose text pane has a shadow pane.
 * @param pActor layout actor
 * @param pPaneName text pane name
 */
void initLayoutTextPaneAnimatorWithShadow(LayoutActor* pActor, const char* pPaneName) {
    const IUseLayout* layout = pActor;
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(layout, pPaneName));
    StringTmp<64> shadowPaneName(pPaneName);
    shadowPaneName.append("Sh");
    auto* shadowTextBox =
        static_cast<nn::ui2d::TextBox*>(findPane(layout, shadowPaneName.cstr()));
    auto* animator = new LayoutTextPaneAnimator(textBox, pActor);
    animator->initEuiLetterAnimCtrlWithShadow(pActor->getLayoutKeeper()->getScreen()->mLayout,
                                              shadowTextBox);
    pActor->initTextPaneAnimator(animator);
}

/**
 * Initializes a layout actor from a parts pane of its parent's layout.
 * @param pActor parts layout actor
 * @param pParentActor parent layout actor
 * @param rInfo layout init info
 * @param pPaneName parts pane name
 * @param pSuffix init file suffix
 */
void initLayoutPartsActor(LayoutActor* pActor, LayoutActor* pParentActor,
                          const LayoutInitInfo& rInfo, const char* pPaneName,
                          const char* pSuffix) {
    initLayoutPartsActorImpl(pActor, pParentActor, rInfo, pPaneName, pSuffix, false);
}

/**
 * Initializes a layout actor from a parts pane of its parent's layout, using localized archives.
 * @param pActor parts layout actor
 * @param pParentActor parent layout actor
 * @param rInfo layout init info
 * @param pPaneName parts pane name
 * @param pSuffix init file suffix
 */
void initLayoutPartsActorLocalized(LayoutActor* pActor, LayoutActor* pParentActor,
                                   const LayoutInitInfo& rInfo, const char* pPaneName,
                                   const char* pSuffix) {
    initLayoutPartsActorImpl(pActor, pParentActor, rInfo, pPaneName, pSuffix, true);
}

/**
 * Creates and initializes the audio keeper of a layout parts actor.
 * @param pActor layout actor
 * @param rInfo layout init info
 * @param rName audio user name
 */
void initLayoutPartsAudioKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                                const sead::SafeString& rName) {
    auto* audioKeeper = new AudioKeeper();
    audioKeeper->init(rInfo.getAudioDirector(), rName.cstr(), rName.cstr(), nullptr, nullptr,
                      nullptr, "サブ");
    pActor->initAudioKeeper(audioKeeper);
}
}  // namespace al
