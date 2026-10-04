#pragma once

#include "Util/IUseRigidBodyCollision.hpp"

namespace al {
    class CollisionPartsFilterActor;
    class LiveActor;
};  // namespace al

/**
 * Default IUseRigidBodyCollision implementation: strike-sphere checks against the scene
 * collision, ignoring the owning actor's own collision parts.
 */
class RigidBodyCollisionImpl : public IUseRigidBodyCollision {
public:
    RigidBodyCollisionImpl(const al::LiveActor* pActor);

    bool checkStrikeSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                           f32 radius) override;
    u32 getHitNum(const al::IUseCollision* pCollision) const override;
    const sead::Vector3f& getHitPosition(const al::IUseCollision* pCollision,
                                         u32 index) const override;
    const sead::Vector3f* getHitNormal(const al::IUseCollision* pCollision,
                                       u32 index) const override;
    f32 getHitOverlap(const al::IUseCollision* pCollision, u32 index) const override;

private:
    al::CollisionPartsFilterActor* mPartsFilter = nullptr;
};

static_assert(sizeof(RigidBodyCollisionImpl) == 0x10);
