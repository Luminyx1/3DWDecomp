#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class ClippingActorInfo;
class ClippingJudge;
class PlacementId;

class ClippingInfoGroup {
public:
    ClippingInfoGroup(s32 maxInfos);

    void registerInfo(ClippingActorInfo* pInfo);
    void setGroupId(const ActorInitInfo& rInfo);
    bool isEqualGroupId(const ActorInitInfo& rInfo) const;
    bool judgeClippingAll(const ClippingJudge* pJudge) const;
    void startClippedAll();
    void endClippedAll();

    s32 mMaxInfos;
    s32 mNumInfos = 0;
    ClippingActorInfo** mInfos = nullptr;
    PlacementId* mGroupId;
    bool mIsClipped = false;
};
}  // namespace al
