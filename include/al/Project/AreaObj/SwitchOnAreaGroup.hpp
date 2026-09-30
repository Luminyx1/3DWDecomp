#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObjGroup;
class IScenarioCompleteChecker;
class LiveActor;

class SwitchOnAreaGroup {
public:
    SwitchOnAreaGroup(AreaObjGroup* pGroup);

    void update(const sead::Vector3f* pPositions, s32 num, bool isDisasterMode);
    void update(const sead::Vector3f& rPos);
    void endInit(IScenarioCompleteChecker* pChecker);

    AreaObjGroup* mGroup;
};

SwitchOnAreaGroup* tryCreateSwitchOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo);
}  // namespace al
