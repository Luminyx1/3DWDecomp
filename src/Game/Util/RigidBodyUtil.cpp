#include "Util/RigidBodyUtil.hpp"

#include <nerd/nerdMath.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>

#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Project/Model/SimpleModelG3D.hpp"
#include "Util/RigidBodyCore.hpp"

namespace {

    void createCollisionSphereChildJoints(RBCollisionSphereArray* pSpheres,
                                          const al::LiveActor* pActor,
                                          const nn::g3d::SkeletonObj* pSkeleton, s32 jointIndex);

    /**
     * Inverts a 3x3 matrix; the result is left untouched if the matrix is singular.
     * @param pOut Receives the inverse.
     * @param rMtx Matrix to invert.
     */
    void calcInverse(sead::Matrix33f* pOut, const sead::Matrix33f& rMtx) {
        f32 c00 = rMtx.m[1][1] * rMtx.m[2][2] - rMtx.m[2][1] * rMtx.m[1][2];
        f32 c01 = rMtx.m[1][2] * rMtx.m[2][0] - rMtx.m[2][2] * rMtx.m[1][0];
        f32 c02 = rMtx.m[2][1] * rMtx.m[1][0] - rMtx.m[1][1] * rMtx.m[2][0];
        f32 det = rMtx.m[0][0] * c00 + rMtx.m[0][1] * c01 + rMtx.m[0][2] * c02;

        if (det == 0.0f) {
            return;
        }

        f32 invDet = 1.0f / det;
        pOut->m[0][0] = c00 * invDet;
        pOut->m[0][1] = -(rMtx.m[2][2] * rMtx.m[0][1] - rMtx.m[2][1] * rMtx.m[0][2]) * invDet;
        pOut->m[0][2] = (rMtx.m[1][2] * rMtx.m[0][1] - rMtx.m[1][1] * rMtx.m[0][2]) * invDet;
        pOut->m[1][0] = -(rMtx.m[2][2] * rMtx.m[1][0] - rMtx.m[1][2] * rMtx.m[2][0]) * invDet;
        pOut->m[1][1] = (rMtx.m[0][0] * rMtx.m[2][2] - rMtx.m[2][0] * rMtx.m[0][2]) * invDet;
        pOut->m[1][2] = -(rMtx.m[0][0] * rMtx.m[1][2] - rMtx.m[1][0] * rMtx.m[0][2]) * invDet;
        pOut->m[2][0] = c02 * invDet;
        pOut->m[2][1] = -(rMtx.m[0][0] * rMtx.m[2][1] - rMtx.m[0][1] * rMtx.m[2][0]) * invDet;
        pOut->m[2][2] = (rMtx.m[0][0] * rMtx.m[1][1] - rMtx.m[0][1] * rMtx.m[1][0]) * invDet;
    }

}  // namespace

namespace RigidBodyUtil {

    /**
     * Creates collision spheres spanning every bone of the actor's skeleton. If no bone is long
     * enough, a single sphere of radius 50 at the actor's origin is created instead.
     * @param pActor Actor with a model.
     * @return Array of collision spheres relative to the actor's translation.
     */
    RBCollisionSphereArray* createCollisionSphereEveryJoints(const al::LiveActor* pActor) {
        const nn::g3d::SkeletonObj* skeleton =
            pActor->getModelKeeper()->getModelCafe()->getModelG3D()->getModelObj()->GetSkeleton();
        s32 jointNum = skeleton->GetBoneCount();
        RBCollisionSphereArray* spheres = new RBCollisionSphereArray();
        spheres->allocBuffer(jointNum, nullptr);

        for (s32 i = 0; i < jointNum; i++) {
            if (skeleton->GetBone(i)->GetParentIndex() >= jointNum) {
                createCollisionSphereChildJoints(spheres, pActor, skeleton, i);
            }
        }

        if (spheres->size() == 0) {
            RBCollisionSphere* sphere = spheres->emplaceBack();
            sphere->mPos.set(0.0f, 0.0f, 0.0f);
            sphere->mRadius = 50.0f;
        }

        return spheres;
    }

};  // namespace RigidBodyUtil

namespace {

    /**
     * Adds a collision sphere between a joint and each of its child joints, recursing into the
     * children. Spheres with a radius of 5 or less are skipped.
     * @param pSpheres Receives the collision spheres.
     * @param pActor Actor the skeleton belongs to.
     * @param pSkeleton Skeleton of the actor's model.
     * @param jointIndex Index of the parent joint.
     */
    void createCollisionSphereChildJoints(RBCollisionSphereArray* pSpheres,
                                          const al::LiveActor* pActor,
                                          const nn::g3d::SkeletonObj* pSkeleton, s32 jointIndex) {
        sead::Vector3f jointPos;
        al::calcJointPos(&jointPos, pActor, pSkeleton->GetRes()->GetBoneName(jointIndex));

        for (s32 i = 0; i < pSkeleton->GetBoneCount(); i++) {
            if (pSkeleton->GetBone(i)->GetParentIndex() != jointIndex) {
                continue;
            }

            sead::Vector3f childPos;
            al::calcJointPos(&childPos, pActor, pSkeleton->GetRes()->GetBoneName(i));
            f32 radius = nerd::sqrt((childPos - jointPos).squaredLength()) * 0.5f;

            if (radius > 5.0f) {
                RBCollisionSphere* sphere = pSpheres->emplaceBack();
                sead::Vector3f center = (jointPos + childPos) * 0.5f;
                sphere->mPos = center - al::getTrans(pActor);
                sphere->mRadius = radius;
            }

            createCollisionSphereChildJoints(pSpheres, pActor, pSkeleton, i);
        }
    }

}  // namespace

namespace RigidBodyUtil {

    /**
     * Moves the collision spheres so that their volume-weighted center lies at the origin.
     * @param pSpheres Collision spheres to adjust.
     * @param num Number of collision spheres.
     * @param pCenter Receives the previous center of gravity.
     */
    void adjustCenterOfGravity(RBCollisionSphere* pSpheres, s32 num, sead::Vector3f* pCenter) {
        pCenter->set(0.0f, 0.0f, 0.0f);
        f32 totalMass = 0.0f;

        for (s32 i = 0; i < num; i++) {
            f32 radius = pSpheres[i].mRadius;
            f32 mass = 4.0f * sead::Mathf::pi() * radius * radius * radius / 3.0f;
            *pCenter += pSpheres[i].mPos * mass;
            totalMass += mass;
        }

        *pCenter *= 1.0f / totalMass;

        for (s32 i = 0; i < num; i++) {
            pSpheres[i].mPos -= *pCenter;
        }
    }

    /**
     * Creates a rigid body from collision spheres.
     * @param pActor Actor whose collision is used, or nullptr.
     * @param pBodyCollision Collision query interface for the body's spheres.
     * @param pSpheres Collision spheres relative to the center of gravity.
     * @param num Number of collision spheres.
     * @param rCenter Center of gravity relative to the body's origin.
     * @param scale Scale applied to the bounding radius of the spheres.
     * @return The new rigid body.
     */
    RigidBodyCore* createRigidBody(const al::LiveActor* pActor,
                                   IUseRigidBodyCollision* pBodyCollision,
                                   const RBCollisionSphere* pSpheres, s32 num,
                                   const sead::Vector3f& rCenter, f32 scale) {
        RigidBodyCore* body = new RigidBodyCore(pBodyCollision, pActor);
        RBCollisionSphereHolder* holder = new RBCollisionSphereHolder(pSpheres, num);
        body->setSphereHolder(holder);
        body->setCenterOffset(rCenter);
        f32 radius = 0.0f;

        for (u32 i = 0; i < holder->getNum(); i++) {
            f32 sphereRadius =
                nerd::sqrt(holder->getPos(i).squaredLength()) + holder->getRadius(i);

            if (radius < sphereRadius) {
                radius = sphereRadius;
            }
        }

        body->setRadius(radius * scale);
        return body;
    }

    /**
     * Creates a rigid body with collision spheres spanning the bones of the actor's skeleton.
     * @param pActor Actor with a model.
     * @param pBodyCollision Collision query interface for the body's spheres.
     * @param scale Scale applied to the bounding radius of the spheres.
     * @return The new rigid body.
     */
    RigidBodyCore* createRigidBodyWithJointCollisionSphere(const al::LiveActor* pActor,
                                                           IUseRigidBodyCollision* pBodyCollision,
                                                           f32 scale) {
        RBCollisionSphereArray* spheres = createCollisionSphereEveryJoints(pActor);
        sead::Vector3f center;
        adjustCenterOfGravity(spheres->front(), spheres->size(), &center);
        return createRigidBody(pActor, pBodyCollision, spheres->front(), spheres->size(), center,
                               scale);
    }

    /**
     * Calculates the inertia tensor of a cuboid.
     * @param pTensor Receives the inertia tensor.
     * @param mass Mass of the cuboid.
     * @param sizeX Size along the x axis.
     * @param sizeY Size along the y axis.
     * @param sizeZ Size along the z axis.
     */
    void calcInertiaTensorOfQuadraticPrism(sead::Matrix33f* pTensor, f32 mass, f32 sizeX,
                                           f32 sizeY, f32 sizeZ) {
        f32 scale = mass / 12.0f;
        f32 xx = sizeX * sizeX;
        f32 yy = sizeY * sizeY;
        f32 zz = sizeZ * sizeZ;
        pTensor->m[0][0] = scale * (yy + zz);
        pTensor->m[0][1] = 0.0f;
        pTensor->m[0][2] = 0.0f;
        pTensor->m[1][0] = 0.0f;
        pTensor->m[1][1] = scale * (xx + zz);
        pTensor->m[1][2] = 0.0f;
        pTensor->m[2][0] = 0.0f;
        pTensor->m[2][1] = 0.0f;
        pTensor->m[2][2] = scale * (xx + yy);
    }

    /**
     * Rotates a body-local inertia tensor into world space.
     * @param pTensor Receives the rotated inertia tensor.
     * @param rLocalTensor Inertia tensor in body-local space.
     * @param rQuat Rotation of the body.
     */
    void updateInertiaTensor(sead::Matrix33f* pTensor, const sead::Matrix33f& rLocalTensor,
                             const sead::Quatf& rQuat) {
        sead::Matrix33f rotMtx;
        rotMtx.fromQuat(rQuat);
        sead::Matrix33f invRotMtx;
        calcInverse(&invRotMtx, rotMtx);
        sead::Matrix33f mtx;
        mtx.setMul(rotMtx, rLocalTensor);
        pTensor->setMul(mtx, invRotMtx);
    }

};  // namespace RigidBodyUtil

/**
 * @return Number of collision spheres.
 */
u32 RBCollisionSphereHolder::getNum() const {
    return mNum;
}

/**
 * @param index Sphere index.
 * @return Position of the collision sphere relative to the center of gravity.
 */
const sead::Vector3f& RBCollisionSphereHolder::getPos(u32 index) const {
    return mSpheres[index].mPos;
}

/**
 * @param index Sphere index.
 * @return Radius of the collision sphere.
 */
f32 RBCollisionSphereHolder::getRadius(u32 index) const {
    return mSpheres[index].mRadius;
}
