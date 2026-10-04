#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Util/IUseRigidBodyCollisionSphereList.hpp"

namespace al {
    class LiveActor;
};  // namespace al

class IUseRigidBodyCollision;
class RigidBodyCore;

/**
 * Collision sphere of a rigid body, in body-local space.
 */
struct RBCollisionSphere {
    sead::Vector3f mPos;
    f32 mRadius;
};

static_assert(sizeof(RBCollisionSphere) == 0x10);

typedef sead::ObjArray<RBCollisionSphere> RBCollisionSphereArray;

/**
 * IUseRigidBodyCollisionSphereList over a plain array of collision spheres.
 */
class RBCollisionSphereHolder : public IUseRigidBodyCollisionSphereList {
public:
    RBCollisionSphereHolder(const RBCollisionSphere* pSpheres, s32 num)
        : mNum(num), mSpheres(pSpheres) {}

    u32 getNum() const override;
    const sead::Vector3f& getPos(u32 index) const override;
    f32 getRadius(u32 index) const override;

private:
    s32 mNum;
    const RBCollisionSphere* mSpheres;
};

static_assert(sizeof(RBCollisionSphereHolder) == 0x18);

namespace RigidBodyUtil {
    RBCollisionSphereArray* createCollisionSphereEveryJoints(const al::LiveActor* pActor);
    void adjustCenterOfGravity(RBCollisionSphere* pSpheres, s32 num, sead::Vector3f* pCenter);
    RigidBodyCore* createRigidBody(const al::LiveActor* pActor,
                                   IUseRigidBodyCollision* pBodyCollision,
                                   const RBCollisionSphere* pSpheres, s32 num,
                                   const sead::Vector3f& rCenter, f32 scale);
    RigidBodyCore* createRigidBodyWithJointCollisionSphere(const al::LiveActor* pActor,
                                                           IUseRigidBodyCollision* pBodyCollision,
                                                           f32 scale);
    void calcInertiaTensorOfQuadraticPrism(sead::Matrix33f* pTensor, f32 mass, f32 sizeX,
                                           f32 sizeY, f32 sizeZ);
    void updateInertiaTensor(sead::Matrix33f* pTensor, const sead::Matrix33f& rLocalTensor,
                             const sead::Quatf& rQuat);
};  // namespace RigidBodyUtil
