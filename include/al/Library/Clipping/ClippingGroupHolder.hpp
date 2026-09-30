#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class ClippingActorInfo;
class ClippingInfoGroup;
class ClippingJudge;

class ClippingGroupHolder {
public:
    ClippingGroupHolder();

    void update(const ClippingJudge* pJudge);
    void createAndAdd(ClippingActorInfo* pInfo, const ActorInitInfo& rInfo, s32 maxInfos);
    ClippingInfoGroup* tryFindGroup(const ActorInitInfo& rInfo);

    s32 mNumGroups = 0;
    ClippingInfoGroup** mGroups;
};
}  // namespace al
