#pragma once

#include <basis/seadTypes.h>

#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
class ActionBgmCtrl;
class ActionEffectCtrl;
class ActionSeCtrl;
class HitReactionKeeper;
class IUseAudioKeeper;
class IUseEffectKeeper;
class LayoutKeeper;

class LayoutActionKeeper {
public:
    struct PaneGroupInfo {
        LayoutPaneGroup* mPaneGroup;
        ActionEffectCtrl* mEffectCtrl;
        ActionSeCtrl* mSeCtrl;
        ActionBgmCtrl* mBgmCtrl;
    };

    LayoutActionKeeper(LayoutKeeper* pLayoutKeeper, IUseAudioKeeper* pAudioKeeper,
                       IUseEffectKeeper* pEffectKeeper);

    bool startAction(const char* pActionName, const char* pGroupName);
    PaneGroupInfo* findPaneGroupInfo(const char* pGroupName) const;
    void update();
    void setMainGroupName(const char* pGroupName);
    LayoutPaneGroup* getLayoutPaneGroup(const char* pGroupName) const;

    void setHitReactionKeeper(HitReactionKeeper* pKeeper) { mHitReactionKeeper = pKeeper; }

private:
    PaneGroupInfo* findPaneGroupInfoByName(const char* pGroupName) const {
        for (s32 i = 0; i < mPaneGroupNum; i++) {
            if (isEqualString(mPaneGroupInfos[i].mPaneGroup->getGroupName(), pGroupName)) {
                return &mPaneGroupInfos[i];
            }
        }
        return nullptr;
    }

    bool mIsActionStarted = false;
    PaneGroupInfo* mPaneGroupInfos = nullptr;
    s32 mPaneGroupNum = 0;
    const char* mMainGroupName = nullptr;
    HitReactionKeeper* mHitReactionKeeper = nullptr;
};
}  // namespace al
