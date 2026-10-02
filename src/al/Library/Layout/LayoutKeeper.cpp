#include "Library/Layout/LayoutKeeper.hpp"

#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <gfx/seadDrawContext.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <nn/ui2d/ui2d_ArcExtractor.h>
#include <nn/ui2d/ui2d_Group.h>
#include <nn/ui2d/ui2d_Layout.h>

#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Library/Layout/LayoutResource.hpp"
#include "Library/Message/CustomTagProcessor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates an empty layout keeper.
 */
LayoutKeeper::LayoutKeeper() = default;

/**
 * Reinitializes the shaders of the screen's layout resource.
 */
void LayoutKeeper::reinitializeShader() {
    if (mScreen == nullptr) {
        return;
    }

    nn::ui2d::Layout* layout = mScreen->mLayout;

    if (layout == nullptr) {
        return;
    }

    nn::ui2d::ResourceAccessor* accessor = layout->GetResourceAccessor();

    if (accessor == nullptr) {
        return;
    }

    auto* resource = eui::DynamicCast<eui::MultiArcResourceAccessor>(accessor);

    if (resource == nullptr) {
        return;
    }

    static_cast<LayoutResource*>(resource)->reinitializeShaders(
        reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()));
}

/**
 * Sets the screen the layout belongs to.
 * @param pScreen screen
 */
void LayoutKeeper::initScreen(eui::Screen* pScreen) {
    mScreen = pScreen;
}

/**
 * Creates a pane group for every layout group and registers the animations found in the
 * resource's archives.
 * @param pLayout layout
 * @param pResource layout resource
 */
void LayoutKeeper::initLayout(nn::ui2d::Layout* pLayout, LayoutResource* pResource) {
    mLayout = pLayout;
    nn::ui2d::GroupContainer::List& groups = pLayout->GetGroupContainer()->mGroups;
    mGroupNum = groups.size();

    if (mGroupNum == 0) {
        return;
    }

    mGroups = new LayoutPaneGroup*[mGroupNum];
    auto groupIt = groups.begin();

    for (s32 i = 0; i < mGroupNum; i++) {
        mGroups[i] = new LayoutPaneGroup(groupIt->mName);
        ++groupIt;
    }

    eui::MultiArcResourceAccessor::ArchiveList& archives = pResource->getArchiveList();

    for (auto it = archives.begin(); it != archives.end(); ++it) {
        const void* archive = it->mExtractor.m_pArchiveBlockHeader;
        nn::ui2d::ArcExtractor extractor;
        extractor.PrepareArchive(archive);
        nn::ui2d::ArcEntry entry;
        s32 entryId = 0;
        StringTmp<128> pattern("*%s_*.bflan", mLayout->GetName());

        for (void* file = extractor.GetFileFast(nullptr, 0);
             extractor.ReadEntry(&entryId, &entry, 1) != 0;
             file = extractor.GetFileFast(nullptr, entryId)) {
            if (!isMatchString(entry.name, MatchStr(pattern.cstr()))) {
                continue;
            }

            nn::ui2d::AnimResource animResource;
            animResource.Set(file);
            const char* tagName = animResource.GetTagName();
            u16 groupNum = animResource.GetGroupCount();

            if (groupNum == 0) {
                continue;
            }

            const nn::ui2d::ResAnimationGroup* animGroups = animResource.GetGroupArray();

            for (s32 j = 0; j < groupNum; j++) {
                getGroup(animGroups[j].name)->pushAnimName(tagName);
            }
        }
    }

    for (s32 i = 0; i < mGroupNum; i++) {
        mGroups[i]->createAnimator(mLayout);
    }
}

/**
 * Sets the draw info used to draw the layout.
 * @param pDrawInfo draw info
 */
void LayoutKeeper::initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo) {
    mDrawInfo = pDrawInfo;
}

/**
 * Sets the tag processor of the layout.
 * @param pTagProcessor tag processor
 */
void LayoutKeeper::initTagProcessor(CustomTagProcessor* pTagProcessor) {
    mTagProcessor = pTagProcessor;
    mLayout->SetTagProcessor(pTagProcessor);
}

/**
 * Finds a pane group by name.
 * @param pGroupName group name
 * @return the group, or nullptr
 */
LayoutPaneGroup* LayoutKeeper::getGroup(const char* pGroupName) const {
    for (s32 i = 0; i < mGroupNum; i++) {
        if (isEqualString(pGroupName, mGroups[i]->getGroupName())) {
            return mGroups[i];
        }
    }

    return nullptr;
}

/**
 * Returns a pane group by index.
 * @param index group index
 * @return the group
 */
LayoutPaneGroup* LayoutKeeper::getGroup(s32 index) const {
    return mGroups[index];
}

/**
 * Returns the number of pane groups.
 * @return number of groups
 */
s32 LayoutKeeper::getGroupNum() const {
    return mGroupNum;
}

/**
 * Advances the layout's animations.
 * @param isRecursive whether to let the screen update all of its animators
 */
void LayoutKeeper::calcAnim(bool isRecursive) {
    if (isRecursive) {
        if (mScreen != nullptr) {
            mScreen->updateAnimator_();
        }

        return;
    }

    if (mScreen != nullptr) {
        requestCaptureRecursive(mLayout->GetRootPane());
    }

    s32 groupNum = mGroupNum;

    for (s32 i = 0; i < groupNum; i++) {
        mGroups[i]->animate(false);
    }
}

/**
 * Calculates and draws the layout.
 */
void LayoutKeeper::draw() {
    mLayout->Calculate(*mDrawInfo);
    mLayout->Draw(*mDrawInfo, *mDrawContext->getCommandBuffer());
}
}  // namespace al
