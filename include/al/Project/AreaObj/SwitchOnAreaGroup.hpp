#pragma once

#include <math/seadVector.h>

namespace al {
    class ActorInitInfo;
    class AreaObjGroup;
    class IScenarioCompleteChecker;
    class LiveActor;

    /// Turns on the SwitchAreaOn switch of the areas of a group that are entered.
    class SwitchOnAreaGroup {
    public:
        SwitchOnAreaGroup(AreaObjGroup* pAreaGroup);

        void update(const sead::Vector3f* pPoints, s32 numPoints, bool isIgnoreDisasterCamera);
        void update(const sead::Vector3f& rPos);
        void endInit(IScenarioCompleteChecker* pChecker);

        AreaObjGroup* mAreaGroup;   // _0
    };

    SwitchOnAreaGroup* tryCreateSwitchOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo);
};
