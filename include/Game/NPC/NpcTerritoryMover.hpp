#pragma once

#include <container/seadObjArray.h>
#include <math/seadVector.h>

namespace al { class LiveActor; }

class NpcTerritoryMover {
public:
    NpcTerritoryMover(const al::LiveActor* pActor, float radius, float spacing);
    void decideNextTargetPos();
    bool isReachedTarget(const al::LiveActor* pActor, float distance) const;
    void calculateTargets();

private:
    struct Target {
        sead::Vector3f mPosition;
        bool mIsValidated;
    };

    float mRadius;
    sead::Vector3f mCenter;
    sead::Vector3f mTargetPosition;
    float mSpacing;
    const al::LiveActor* mActor;
    sead::ObjArray<Target> mTargets;
};
