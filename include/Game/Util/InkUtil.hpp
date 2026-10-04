#pragma once

#include <math/seadVector.h>

namespace al {
    class IUseCollision;
    class LiveActor;
};  // namespace al

namespace InkUtil {
    bool isInInkLimitArrow(const al::LiveActor* pActor);
    bool isInInkLimitArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                           f32 radius);
    bool isInInkLimitArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                           const sead::Vector3f& rDir);
    bool isInInkLimitSphere(const al::LiveActor* pActor);
    bool isInInkLimitSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                            f32 radius);
};  // namespace InkUtil
