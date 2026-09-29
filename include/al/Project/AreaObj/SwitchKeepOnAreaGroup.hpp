#pragma once

#include <math/seadVector.h>

namespace al {
    class ActorInitInfo;
    class AreaObj;
    class AreaObjGroup;
    class LiveActor;

    /// Keeps the SwitchAreaOn switch of the areas of a group on while they are occupied.
    class SwitchKeepOnAreaGroup {
    public:
        SwitchKeepOnAreaGroup(AreaObjGroup* pAreaGroup);

        void update(const sead::Vector3f* pPoints, s32 numPoints, bool isIgnoreDisasterCamera);
        void update(const sead::Vector3f& rPos);

        AreaObjGroup* mAreaGroup;   // _0
        AreaObj** mOnAreas;         // _8
        s32 mMaxOnAreas;            // _10
        s32 mNumOnAreas;            // _14
    };

    SwitchKeepOnAreaGroup* tryCreateSwitchKeepOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo);
};
