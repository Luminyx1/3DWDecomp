#include "Library/Actor/ActorPoseKeeper.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"

namespace al {
static void rotationAndTranslationFromMatrix(sead::Vector3f* pTrans, sead::Vector3f* pRotate,
                                             const sead::Matrix34f* pMtx) {
    sead::Vector3f rotate;
    pMtx->getRotation(rotate);
    pRotate->set(sead::Mathf::rad2deg(rotate.x), sead::Mathf::rad2deg(rotate.y),
                 sead::Mathf::rad2deg(rotate.z));
    pMtx->getTranslation(*pTrans);
}

sead::Vector3f ActorPoseKeeperBase::sDefaultVelocity = {0.0f, -1.0f, 0.0f};

/**
 * Constructs a pose keeper at the origin.
 */
ActorPoseKeeperBase::ActorPoseKeeperBase() = default;

/**
 * Copies the pose of another pose keeper through its base matrix.
 * @param pOther The pose keeper to copy.
 */
void ActorPoseKeeperBase::copyPose(const ActorPoseKeeperBase* pOther) {
    sead::Matrix34f mtx;
    mtx = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    pOther->calcBaseMtx(&mtx);
    updatePoseMtx(&mtx);
}

/**
 * Constructs a translation, rotation, scale and velocity pose keeper.
 */
ActorPoseKeeperTRSV::ActorPoseKeeperTRSV() = default;

/**
 * Sets the rotation.
 * @param rRotate The rotation in degrees.
 */
void ActorPoseKeeperTRSV::updatePoseRotate(const sead::Vector3f& rRotate) {
    mRotate = rRotate;
}

/**
 * Sets the rotation from a quaternion.
 * @param rQuat The quaternion.
 */
void ActorPoseKeeperTRSV::updatePoseQuat(const sead::Quatf& rQuat) {
    sead::Vector3f rotate;
    rQuat.calcRPY(rotate);
    mRotate = {sead::Mathf::rad2deg(rotate.x), sead::Mathf::rad2deg(rotate.y),
               sead::Mathf::rad2deg(rotate.z)};
}

/**
 * Sets the rotation and translation from a matrix.
 * @param pMtx The matrix.
 */
void ActorPoseKeeperTRSV::updatePoseMtx(const sead::Matrix34f* pMtx) {
    rotationAndTranslationFromMatrix(&mTranslation, &mRotate, pMtx);
}

/**
 * Calculates the base matrix from the rotation and translation.
 * @param pMtx The resulting matrix.
 */
void ActorPoseKeeperTRSV::calcBaseMtx(sead::Matrix34f* pMtx) const {
    sead::Vector3f rotate = getRotate() * (sead::numbers::pi / 180.0f);
    pMtx->makeRT(rotate, mTranslation);
}

/**
 * Constructs a translation, rotation, matrix, scale and velocity pose keeper.
 */
ActorPoseKeeperTRMSV::ActorPoseKeeperTRMSV() {
    mMtx = sead::Matrix34f::ident;
}

/**
 * Sets the rotation and updates the matrix.
 * @param rRotate The rotation in degrees.
 */
void ActorPoseKeeperTRMSV::updatePoseRotate(const sead::Vector3f& rRotate) {
    mRotate = rRotate;
    sead::Vector3f rotate = getRotate() * (sead::numbers::pi / 180.0f);
    mMtx.makeRT(rotate, mTranslation);
}

/**
 * Sets the rotation from a quaternion and updates the matrix.
 * @param rQuat The quaternion.
 */
void ActorPoseKeeperTRMSV::updatePoseQuat(const sead::Quatf& rQuat) {
    sead::Vector3f rpy;
    rQuat.calcRPY(rpy);
    mRotate = rpy * (180.0f / sead::numbers::pi);
    sead::Vector3f rotate = getRotate() * (sead::numbers::pi / 180.0f);
    mMtx.makeRT(rotate, mTranslation);
}

/**
 * Sets the matrix, rotation and translation from a matrix.
 * @param pMtx The matrix.
 */
void ActorPoseKeeperTRMSV::updatePoseMtx(const sead::Matrix34f* pMtx) {
    mMtx = *pMtx;
    rotationAndTranslationFromMatrix(&mTranslation, &mRotate, pMtx);
}

/**
 * Copies the matrix.
 * @param pMtx The resulting matrix.
 */
void ActorPoseKeeperTRMSV::calcBaseMtx(sead::Matrix34f* pMtx) const {
    *pMtx = mMtx;
}

/**
 * Constructs a translation, front, scale and velocity pose keeper.
 */
ActorPoseKeeperTFSV::ActorPoseKeeperTFSV() = default;

/**
 * Sets the front direction from a rotation.
 * @param rRotate The rotation in degrees.
 */
void ActorPoseKeeperTFSV::updatePoseRotate(const sead::Vector3f& rRotate) {
    sead::Quatf quat;
    quat.setRPY(sead::Mathf::deg2rad(rRotate.x), sead::Mathf::deg2rad(rRotate.y),
                sead::Mathf::deg2rad(rRotate.z));
    calcQuatFront(&mFront, quat);
}

/**
 * Sets the front direction from a quaternion.
 * @param rQuat The quaternion.
 */
void ActorPoseKeeperTFSV::updatePoseQuat(const sead::Quatf& rQuat) {
    calcQuatFront(&mFront, rQuat);
}

/**
 * Sets the front direction and translation from a matrix.
 * @param pMtx The matrix.
 */
void ActorPoseKeeperTFSV::updatePoseMtx(const sead::Matrix34f* pMtx) {
    pMtx->getBase(mFront, 2);
    pMtx->getBase(mTranslation, 3);
}

/**
 * Calculates the base matrix from the front direction, gravity and translation.
 * @param pMtx The resulting matrix.
 */
void ActorPoseKeeperTFSV::calcBaseMtx(sead::Matrix34f* pMtx) const {
    makeMtxFrontUpPos(pMtx, mFront, -getGravity(), mTranslation);
}

/**
 * Constructs a translation, front, gravity, scale and velocity pose keeper.
 */
ActorPoseKeeperTFGSV::ActorPoseKeeperTFGSV() = default;

/**
 * Sets the front direction and gravity from a rotation.
 * @param rRotate The rotation in degrees.
 */
void ActorPoseKeeperTFGSV::updatePoseRotate(const sead::Vector3f& rRotate) {
    sead::Quatf quat;
    quat.setRPY(sead::Mathf::deg2rad(rRotate.x), sead::Mathf::deg2rad(rRotate.y),
                sead::Mathf::deg2rad(rRotate.z));
    ActorPoseKeeperTFSV::updatePoseQuat(quat);
    calcQuatUp(&mGravity, quat);
    mGravity *= -1.0f;
}

/**
 * Sets the front direction and gravity from a quaternion.
 * @param rQuat The quaternion.
 */
void ActorPoseKeeperTFGSV::updatePoseQuat(const sead::Quatf& rQuat) {
    ActorPoseKeeperTFSV::updatePoseQuat(rQuat);
    calcQuatUp(&mGravity, rQuat);
    mGravity *= -1.0f;
}

/**
 * Sets the front direction, gravity and translation from a matrix.
 * @param pMtx The matrix.
 */
void ActorPoseKeeperTFGSV::updatePoseMtx(const sead::Matrix34f* pMtx) {
    ActorPoseKeeperTFSV::updatePoseMtx(pMtx);
    pMtx->getBase(mGravity, 1);
    mGravity *= -1.0f;
}

/**
 * Calculates the base matrix from the gravity, front direction and translation.
 * @param pMtx The resulting matrix.
 */
void ActorPoseKeeperTFGSV::calcBaseMtx(sead::Matrix34f* pMtx) const {
    makeMtxUpFrontPos(pMtx, -getGravity(), getFront(), mTranslation);
}

/**
 * Constructs a translation, quaternion, scale and velocity pose keeper.
 */
ActorPoseKeeperTQSV::ActorPoseKeeperTQSV() = default;

/**
 * Sets the quaternion from a rotation.
 * @param rRotate The rotation in degrees.
 */
void ActorPoseKeeperTQSV::updatePoseRotate(const sead::Vector3f& rRotate) {
    mQuat.setRPY(sead::Mathf::deg2rad(rRotate.x), sead::Mathf::deg2rad(rRotate.y),
                 sead::Mathf::deg2rad(rRotate.z));
}

/**
 * Sets the quaternion.
 * @param rQuat The quaternion.
 */
void ActorPoseKeeperTQSV::updatePoseQuat(const sead::Quatf& rQuat) {
    mQuat.x = rQuat.x;
    mQuat.y = rQuat.y;
    mQuat.z = rQuat.z;
    mQuat.w = rQuat.w;
}

/**
 * Sets the quaternion and translation from a matrix.
 * @param pMtx The matrix.
 */
void ActorPoseKeeperTQSV::updatePoseMtx(const sead::Matrix34f* pMtx) {
    pMtx->toQuat(mQuat);
    pMtx->getBase(mTranslation, 3);
}

/**
 * Calculates the base matrix from the quaternion and translation.
 * @param pMtx The resulting matrix.
 */
void ActorPoseKeeperTQSV::calcBaseMtx(sead::Matrix34f* pMtx) const {
    pMtx->makeQT(mQuat, mTranslation);
}
}  // namespace al
