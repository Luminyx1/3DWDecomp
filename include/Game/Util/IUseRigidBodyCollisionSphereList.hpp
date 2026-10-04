#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

/**
 * Provides the collision spheres (in body-local space) that RigidBodyCore tests against the
 * world every physics step.
 */
class IUseRigidBodyCollisionSphereList {
public:
    virtual u32 getNum() const = 0;
    virtual const sead::Vector3f& getPos(u32 index) const = 0;
    virtual f32 getRadius(u32 index) const = 0;
};
