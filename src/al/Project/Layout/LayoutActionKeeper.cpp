#include "Project/Layout/LayoutActionKeeper.hpp"

#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Library/LiveActor/HitReactionKeeper.hpp"
#include "Project/Action/Common/ActionBgmCtrl.hpp"
#include "Project/Action/Common/ActionEffectCtrl.hpp"
#include "Project/Action/Common/ActionSeCtrl.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace al {
/**
 * Creates the action controllers for every pane group of a layout.
 * @param pLayoutKeeper layout keeper
 * @param pAudioKeeper audio keeper user, or nullptr
 * @param pEffectKeeper effect keeper user, or nullptr
 */
LayoutActionKeeper::LayoutActionKeeper(LayoutKeeper* pLayoutKeeper, IUseAudioKeeper* pAudioKeeper,
                                       IUseEffectKeeper* pEffectKeeper) {
    mPaneGroupNum = pLayoutKeeper->getGroupNum();

    if (mPaneGroupNum < 1) {
        return;
    }

    mPaneGroupInfos = new PaneGroupInfo[mPaneGroupNum];

    for (s32 i = 0; i < mPaneGroupNum; i++) {
        mPaneGroupInfos[i].mPaneGroup = pLayoutKeeper->getGroup(i);
        mPaneGroupInfos[i].mEffectCtrl =
            pEffectKeeper ? ActionEffectCtrl::tryCreate(pEffectKeeper) : nullptr;
        if (pAudioKeeper) {
            mPaneGroupInfos[i].mSeCtrl = ActionSeCtrl::tryCreate(pAudioKeeper->getAudioKeeper());
            mPaneGroupInfos[i].mBgmCtrl = ActionBgmCtrl::tryCreate(pAudioKeeper->getAudioKeeper());
        } else {
            mPaneGroupInfos[i].mSeCtrl = nullptr;
            mPaneGroupInfos[i].mBgmCtrl = nullptr;
        }
    }
}

/**
 * Starts an action on a pane group.
 * @param pActionName action name
 * @param pGroupName pane group name, or nullptr for the main group
 * @return whether the pane group was found
 */
bool LayoutActionKeeper::startAction(const char* pActionName, const char* pGroupName) {
    PaneGroupInfo* info = findPaneGroupInfo(pGroupName);

    if (!info) {
        return false;
    }

    mIsActionStarted = true;
    info->mPaneGroup->startAnim(pActionName);

    if (info->mEffectCtrl) {
        info->mEffectCtrl->startAction(pActionName);
    }

    if (info->mSeCtrl) {
        info->mSeCtrl->startAction(pActionName);
    }

    if (info->mBgmCtrl) {
        info->mBgmCtrl->startAction(pActionName);
    }

    if (mHitReactionKeeper) {
        mHitReactionKeeper->start(pActionName, nullptr, nullptr, nullptr);
    }

    return true;
}

/**
 * Finds the pane group info of a pane group.
 * @param pGroupName pane group name, or nullptr for the main group
 * @return the pane group info, or nullptr
 */
LayoutActionKeeper::PaneGroupInfo*
LayoutActionKeeper::findPaneGroupInfo(const char* pGroupName) const {
    if (pGroupName) {
        return findPaneGroupInfoByName(pGroupName);
    }

    if (mMainGroupName) {
        return findPaneGroupInfoByName(mMainGroupName);
    }

    return mPaneGroupInfos;
}

/**
 * Updates the action controllers of every playing pane group.
 */
void LayoutActionKeeper::update() {
    for (s32 i = 0; i < mPaneGroupNum; i++) {
        PaneGroupInfo& info = mPaneGroupInfos[i];
        LayoutPaneGroup* paneGroup = info.mPaneGroup;

        if (!paneGroup->isAnimPlaying()) {
            continue;
        }

        f32 frame = paneGroup->getAnimFrame();
        paneGroup->getAnimFrameMax();
        f32 frameRate = paneGroup->getAnimFrameRate();
        paneGroup->isAnimOneTime();

        if (info.mEffectCtrl) {
            info.mEffectCtrl->update(frame, frameRate);
        }

        if (info.mSeCtrl) {
            info.mSeCtrl->update(frame, frameRate);
        }

        if (info.mBgmCtrl) {
            info.mBgmCtrl->update(frame, frameRate);
        }
    }

    mIsActionStarted = false;
}

/**
 * Sets the pane group used when no group name is given.
 * @param pGroupName pane group name
 */
void LayoutActionKeeper::setMainGroupName(const char* pGroupName) {
    mMainGroupName = pGroupName;
}

/**
 * Returns a pane group by name.
 * @param pGroupName pane group name, or nullptr for the main group
 * @return the pane group, or nullptr
 */
LayoutPaneGroup* LayoutActionKeeper::getLayoutPaneGroup(const char* pGroupName) const {
    PaneGroupInfo* info = findPaneGroupInfo(pGroupName);
    return info ? info->mPaneGroup : nullptr;
}
}  // namespace al
