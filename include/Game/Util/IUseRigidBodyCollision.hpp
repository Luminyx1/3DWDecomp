#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class IUseCollision;
};  // namespace al

/**
 * Collision query interface used by RigidBodyCore to test its collision spheres against the
 * world and read back the resulting hits.
 */
class IUseRigidBodyCollision {
public:
    virtual bool checkStrikeSphere(const al::IUseCollision* pCollision,
                                   const sead::Vector3f& rPos, f32 radius) = 0;
    virtual u32 getHitNum(const al::IUseCollision* pCollision) const = 0;
    virtual const sead::Vector3f& getHitPosition(const al::IUseCollision* pCollision,
                                                 u32 index) const = 0;
    virtual const sead::Vector3f* getHitNormal(const al::IUseCollision* pCollision,
                                               u32 index) const = 0;
    virtual f32 getHitOverlap(const al::IUseCollision* pCollision, u32 index) const = 0;
};
