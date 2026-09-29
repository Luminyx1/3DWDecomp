#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
    class LiveActor;

    void setVelocityZero(LiveActor*);

    void resetPosition(LiveActor*, bool);

    void resetPosition(LiveActor*, const sead::Vector3f&, const sead::Vector3f&);

    void rotateQuatYDirDegree(LiveActor*, f32);
    void rotateQuatLocalDirDegree(LiveActor*, s32, f32);
    void rotateQuatLocalDirDegree(LiveActor*, const sead::Quatf&, s32, f32);

    void addVelocityToDirection(LiveActor*, const sead::Vector3f&, f32);

    void scaleVelocity(LiveActor*, f32);

    void faceToTarget(LiveActor*, const sead::Vector3f&);

    bool turnQuatFrontToDirDegreeH(LiveActor*, const sead::Vector3f&, f32);
    bool turnDirectionDegree(const LiveActor*, sead::Vector3f*, const sead::Vector3f&, f32);
};  // namespace al
