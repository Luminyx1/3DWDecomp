#include "Util/RigidBodyCore.hpp"

#include <math/seadQuat.h>
#include <nerd/nerdMath.h>

#include "Library/Math/MathUtil.hpp"
#include "Util/IUseRigidBodyCollision.hpp"
#include "Util/IUseRigidBodyCollisionSphereList.hpp"

/**
 * Creates a rigid body with default physics parameters.
 * @param pBodyCollision Collision query interface for the body's spheres.
 * @param pCollision Collision user passed to the collision queries.
 */
RigidBodyCore::RigidBodyCore(IUseRigidBodyCollision* pBodyCollision,
                             const al::IUseCollision* pCollision)
    : mCollision(pCollision), mBodyCollision(pBodyCollision) {
    initMembers();
}

/**
 * Resets the pose and physics parameters to their defaults and allocates the power buffers.
 */
void RigidBodyCore::initMembers() {
    mPose.mTrans.set(0.0f, 0.0f, 0.0f);
    mPose.mMtx.makeIdentity();
    mStepMtx.makeIdentity();
    mGravity.set(0.0f, -0.2f, 0.0f);
    mRepulsion = 2.0f;
    mRotDamping = 0.999f;
    mPosCorrection = 1.0f;
    mFriction = 0.999f;
    mVelocity.set(0.0f, 0.0f, 0.0f);

    for (s32 i = 0; i < 3; i++) {
        mRotVelocity[i].set(0.0f, 0.0f);
    }

    mMaxSpeed = -1.0f;
    _c4 = false;
    _c8 = nullptr;
    _d0 = nullptr;
    _d8 = nullptr;
    mIsCollided = false;
    mIsSoundTrigger = false;
    mSoundScalar = 0.0f;
    mPower = new RigidBodyPower();
    mRequestPower = new RigidBodyPower();
    mSphereHolder = nullptr;
    mCenterOffset.set(0.0f, 0.0f, 0.0f);
}

/**
 * Places the body and stops all motion.
 * @param pMtx Pose of the body's origin; the center offset is added to its translation.
 */
void RigidBodyCore::initPose(const sead::Matrix34f* pMtx) {
    sead::Vector3f trans;
    pMtx->getTranslation(trans);
    mPose.mTrans = trans + mCenterOffset;
    mPose.mMtx = *pMtx;
    mPose.mMtx.setTranslation(0.0f, 0.0f, 0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);

    for (s32 i = 0; i < 3; i++) {
        mRotVelocity[i].set(0.0f, 0.0f);
    }
}

/**
 * Requests a force for the next physics step.
 * @param rPos World position the force is applied at.
 * @param rForce Force to apply.
 */
void RigidBodyCore::requestPower(const sead::Vector3f& rPos, const sead::Vector3f& rForce) {
    mRequestPower->power(mPose, mRadius, rPos, rForce);
}

/**
 * Splits a force applied at a point into a linear and a rotational contribution.
 * @param rPose Pose of the body the force is applied to.
 * @param radius Body radius; the farther the force line is from the center, the less of it
 *               becomes linear motion.
 * @param rPos World position the force is applied at.
 * @param rForce Force to apply.
 */
void RigidBodyPower::power(const RigidBodyCore::Pose& rPose, f32 radius,
                           const sead::Vector3f& rPos, const sead::Vector3f& rForce) {
    f32 forceLength = nerd::sqrt(rForce.squaredLength());
    if (forceLength == 0.0f) {
        return;
    }

    sead::Vector3f forceDir = rForce * (1.0f / forceLength);
    sead::Vector3f arm = rPos - rPose.mTrans;
    f32 armLengthSq = arm.squaredLength();
    f32 proj = forceDir.dot(arm);
    f32 distSq = armLengthSq - proj * proj;
    f32 dist = nerd::sqrt(distSq < 0.0f ? 0.0f : distSq);
    if (dist == 0.0f) {
        addVec(rForce);
        return;
    }

    f32 invRadius = 1.0f / radius;
    f32 rate = 1.0f - invRadius * dist;
    rate = rate > 1.0f ? 1.0f : rate;
    if (rate < 0.0f) {
        rate = 0.01f;
    }

    if (rate > 0.0f) {
        addVec(rForce * (rate * rate));
    }

    f32 rotRate = invRadius * 0.999f;
    sead::Vector3f armDir = arm * (1.0f / nerd::sqrt(armLengthSq));
    sead::Vector3f tangent = (rForce - armDir * armDir.dot(rForce)) * rotRate;
    addRot(rPose, armDir, tangent);
}

/**
 * Advances the simulation by one frame in four sub-steps.
 */
void RigidBodyCore::update() {
    sead::Vector3f prevVelocity = mVelocity;
    mIsCollided = false;

    for (s32 i = 0; i < 4; i++) {
        startPhysics();
        calcCollisionImpact();
        control();
        addGravity();
        endPhysics();
    }

    calcSoundParam(prevVelocity);
}

/**
 * Integrates position and rotation, re-orthonormalizes the orientation and stores the
 * transform from the new pose back to the previous one.
 */
void RigidBodyCore::startPhysics() {
    sead::Vector3f prevTrans = mPose.mTrans;
    sead::Matrix34f prevMtx = mPose.mMtx;
    mPose.mTrans += mVelocity;

    sead::Vector3f prevAxisX;
    sead::Vector3f prevAxisY;
    sead::Vector3f prevAxisZ;
    prevMtx.getBase(prevAxisX, 0);
    prevMtx.getBase(prevAxisY, 1);
    prevMtx.getBase(prevAxisZ, 2);

    mPose.mMtx.m[0][0] += prevAxisY.x * mRotVelocity[0].x + prevAxisZ.x * mRotVelocity[0].y;
    mPose.mMtx.m[1][0] += prevAxisY.y * mRotVelocity[0].x + prevAxisZ.y * mRotVelocity[0].y;
    mPose.mMtx.m[2][0] += prevAxisY.z * mRotVelocity[0].x + prevAxisZ.z * mRotVelocity[0].y;
    mPose.mMtx.m[0][1] += prevAxisZ.x * mRotVelocity[1].x + prevAxisX.x * mRotVelocity[1].y;
    mPose.mMtx.m[1][1] += prevAxisZ.y * mRotVelocity[1].x + prevAxisX.y * mRotVelocity[1].y;
    mPose.mMtx.m[2][1] += prevAxisZ.z * mRotVelocity[1].x + prevAxisX.z * mRotVelocity[1].y;
    mPose.mMtx.m[0][2] += prevAxisX.x * mRotVelocity[2].x + prevAxisY.x * mRotVelocity[2].y;
    mPose.mMtx.m[1][2] += prevAxisX.y * mRotVelocity[2].x + prevAxisY.y * mRotVelocity[2].y;
    mPose.mMtx.m[2][2] += prevAxisX.z * mRotVelocity[2].x + prevAxisY.z * mRotVelocity[2].y;

    sead::Vector3f axisX;
    sead::Vector3f axisY;
    sead::Vector3f axisZ;
    f32 orthoSum;
    do {
        mPose.mMtx.getBase(axisX, 0);
        f32 invLength = 1.0f / nerd::sqrt(axisX.squaredLength());
        mPose.mMtx.m[0][0] *= invLength;
        mPose.mMtx.m[1][0] *= invLength;
        mPose.mMtx.m[2][0] *= invLength;
        mPose.mMtx.getBase(axisY, 1);
        invLength = 1.0f / nerd::sqrt(axisY.squaredLength());
        mPose.mMtx.m[0][1] *= invLength;
        mPose.mMtx.m[1][1] *= invLength;
        mPose.mMtx.m[2][1] *= invLength;
        mPose.mMtx.getBase(axisZ, 2);
        invLength = 1.0f / nerd::sqrt(axisZ.squaredLength());
        mPose.mMtx.m[0][2] *= invLength;
        mPose.mMtx.m[1][2] *= invLength;
        mPose.mMtx.m[2][2] *= invLength;

        mPose.mMtx.getBase(axisZ, 2);
        mPose.mMtx.getBase(axisY, 1);
        mPose.mMtx.getBase(axisX, 0);
        sead::Vector3f crossYZ = axisY.cross(axisZ);
        sead::Vector3f crossZX = axisZ.cross(axisX);
        sead::Vector3f crossXY = axisX.cross(axisY);

        orthoSum = nerd::sqrt(crossYZ.squaredLength());
        crossYZ *= 1.0f / orthoSum;
        mPose.mMtx.m[0][0] = (mPose.mMtx.m[0][0] + crossYZ.x) * 0.5f;
        mPose.mMtx.m[1][0] = (mPose.mMtx.m[1][0] + crossYZ.y) * 0.5f;
        mPose.mMtx.m[2][0] = (mPose.mMtx.m[2][0] + crossYZ.z) * 0.5f;
        f32 length = nerd::sqrt(crossZX.squaredLength());
        orthoSum += length;
        crossZX *= 1.0f / length;
        mPose.mMtx.m[0][1] = (mPose.mMtx.m[0][1] + crossZX.x) * 0.5f;
        mPose.mMtx.m[1][1] = (mPose.mMtx.m[1][1] + crossZX.y) * 0.5f;
        mPose.mMtx.m[2][1] = (mPose.mMtx.m[2][1] + crossZX.z) * 0.5f;
        length = nerd::sqrt(crossXY.squaredLength());
        orthoSum += length;
        crossXY *= 1.0f / length;
        mPose.mMtx.m[0][2] = (mPose.mMtx.m[0][2] + crossXY.x) * 0.5f;
        mPose.mMtx.m[1][2] = (mPose.mMtx.m[1][2] + crossXY.y) * 0.5f;
        mPose.mMtx.m[2][2] = (mPose.mMtx.m[2][2] + crossXY.z) * 0.5f;
    } while (orthoSum < 2.999f);

    sead::Matrix34f invMtx = mPose.mMtx;
    invMtx.setTranslation(mPose.mTrans);
    invMtx.setInverse(invMtx);
    prevMtx.setTranslation(prevTrans);
    mStepMtx = prevMtx * invMtx;
    mPower->zero();
}

/**
 * Tests every collision sphere against the world and applies an impact for each hit.
 */
void RigidBodyCore::calcCollisionImpact() {
    if (mSphereHolder == nullptr) {
        return;
    }

    for (u32 i = 0; i < mSphereHolder->getNum(); i++) {
        sead::Vector3f pos = mSphereHolder->getPos(i);
        pos.rotate(mPose.mMtx);
        pos.x += mPose.mTrans.x;
        pos.y += mPose.mTrans.y;
        pos.z += mPose.mTrans.z;

        if (!mBodyCollision->checkStrikeSphere(mCollision, pos, mSphereHolder->getRadius(i))) {
            continue;
        }

        for (u32 j = 0; j < mBodyCollision->getHitNum(mCollision); j++) {
            const sead::Vector3f& hitPos = mBodyCollision->getHitPosition(mCollision, j);
            const sead::Vector3f* hitNormal = mBodyCollision->getHitNormal(mCollision, j);
            f32 overlap = mBodyCollision->getHitOverlap(mCollision, j);
            sead::Vector3f contact = hitPos - *hitNormal * overlap;
            impact(contact, *mBodyCollision->getHitNormal(mCollision, j),
                   mBodyCollision->getHitOverlap(mCollision, j));
            mIsCollided = true;
        }
    }
}

/**
 * Adds one sub-step's share of the gravity.
 */
void RigidBodyCore::addGravity() {
    sead::Vector3f gravity = mGravity;
    gravity *= 0.25f;
    mPower->addVec(gravity);
}

/**
 * Applies the accumulated powers to position, velocity and rotation rate, with damping and the
 * speed limit.
 */
void RigidBodyCore::endPhysics() {
    mPose.mTrans = mPower->calcPos() + mRequestPower->calcPos() + mPose.mTrans;
    mVelocity = mPower->calcVec() + mRequestPower->calcVec() + mVelocity * 0.999f;

    f32 maxSpeed = mMaxSpeed;
    if (maxSpeed > 0.0f && mVelocity.squaredLength() > maxSpeed * maxSpeed) {
        f32 speed = nerd::sqrt(mVelocity.squaredLength());
        if (speed > 0.0f) {
            mVelocity *= maxSpeed / speed;
        }
    }

    for (s32 i = 0; i < 3; i++) {
        mRotVelocity[i] = mPower->calcRot(i) + mRequestPower->calcRot(i) +
                          mRotVelocity[i] * mRotDamping;
    }

    mRequestPower->zero();
}

/**
 * Detects a sharp change of the moving direction for the impact sound.
 * @param prevVelocity Velocity at the start of the frame.
 */
void RigidBodyCore::calcSoundParam(sead::Vector3f prevVelocity) {
    f32 prevSpeed;
    f32 speed;
    sead::Vector3f prevDir;
    sead::Vector3f dir;
    al::separateScalarAndDirection(&prevSpeed, &prevDir, prevVelocity);
    al::separateScalarAndDirection(&speed, &dir, mVelocity);

    sead::Vector3f dirDiff = -prevDir + dir;
    if (!al::isNearZero(dirDiff, 0.001f)) {
        al::normalize(&dirDiff);
    }

    if (prevDir.dot(dirDiff) * dirDiff.dot(dir) < -0.1) {
        mIsSoundTrigger = true;
        mSoundScalar = speed;
    } else {
        mIsSoundTrigger = false;
        mSoundScalar = 0.0f;
    }
}

/**
 * @return Position of the center of gravity.
 */
const sead::Vector3f& RigidBodyCore::getPos() const {
    return mPose.mTrans;
}

/**
 * Calculates the pose of the body's origin (the center of gravity minus the center offset).
 * @param pMtx Output matrix.
 */
void RigidBodyCore::calcPoseMtx(sead::Matrix34f* pMtx) const {
    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 3; j++) {
            pMtx->m[i][j] = mPose.mMtx.m[i][j];
        }
    }

    sead::Vector3f offset;
    offset.setRotated(mPose.mMtx, -mCenterOffset);
    pMtx->m[0][3] = offset.x + mPose.mTrans.x;
    pMtx->m[1][3] = offset.y + mPose.mTrans.y;
    pMtx->m[2][3] = offset.z + mPose.mTrans.z;
}

/**
 * @param rGravity Gravity added per frame.
 */
void RigidBodyCore::setGravity(const sead::Vector3f& rGravity) {
    mGravity.set(rGravity);
}

/**
 * @param pSphereHolder Collision sphere list, or nullptr to disable collision.
 */
void RigidBodyCore::setSphereHolder(const IUseRigidBodyCollisionSphereList* pSphereHolder) {
    mSphereHolder = pSphereHolder;
}

/**
 * Resolves one collision hit: pushes the body out along the normal and applies friction.
 * @param rPos Contact position.
 * @param rNormal Hit normal.
 * @param overlap Penetration depth.
 */
void RigidBodyCore::impact(const sead::Vector3f& rPos, const sead::Vector3f& rNormal,
                           f32 overlap) {
    sead::Vector3f move = mStepMtx * rPos - rPos;
    f32 normalMove = move.dot(rNormal);
    sead::Vector3f normalPart = rNormal * normalMove;
    sead::Vector3f push;
    f32 rate;
    f32 depth;
    if (normalMove < overlap || normalMove == 0.0f) {
        push = rNormal * overlap;
        rate = -1.0f;
        depth = overlap;
    } else {
        rate = overlap / normalMove;
        push = normalPart;
        depth = normalMove;
    }

    sead::Vector3f tangent = move - normalPart;
    f32 friction = mFriction;
    if (depth < 0.03f) {
        friction = depth / 0.03f * friction;
    }

    sead::Vector3f force = tangent * friction + push * mRepulsion;
    if (rate < 0.0f) {
        mPower->addPos(push);
    } else {
        mPower->power(mPose, mRadius, rPos, force);
        mPower->addPos((tangent + push) * rate * mPosCorrection);
    }
}

/**
 * Records a positional correction.
 * @param rPos Correction to record.
 */
void RigidBodyPower::addPos(const sead::Vector3f& rPos) {
    mPos.add(rPos);
}

/**
 * Clears all contributions.
 */
void RigidBodyPower::zero() {
    mPos.mNeg.set(0.0f, 0.0f, 0.0f);
    mPos.mPos.set(0.0f, 0.0f, 0.0f);
    zeroVec();
    zeroRot();
}

/**
 * Records a velocity change.
 * @param rVec Velocity change to record.
 */
void RigidBodyPower::addVec(const sead::Vector3f& rVec) {
    mVec.add(rVec);
}

/**
 * Creates an empty power buffer.
 */
RigidBodyPower::RigidBodyPower() {
    zero();
}

/**
 * Clears the velocity contributions.
 */
void RigidBodyPower::zeroVec() {
    mVec.mNeg.set(0.0f, 0.0f, 0.0f);
    mVec.mPos.set(0.0f, 0.0f, 0.0f);
}

/**
 * Clears the rotation contributions.
 */
void RigidBodyPower::zeroRot() {
    for (s32 i = 0; i < 3; i++) {
        mRot[i].mNeg.set(0.0f, 0.0f);
        mRot[i].mPos.set(0.0f, 0.0f);
    }
}

/**
 * Records the rotation that turns the lever arm towards the force, as the change of every
 * rotation axis projected onto the other two axes.
 * @param rPose Pose of the body.
 * @param rArmDir Normalized direction from the center of gravity to the force position.
 * @param rForce Tangential part of the force.
 */
void RigidBodyPower::addRot(const RigidBodyCore::Pose& rPose, const sead::Vector3f& rArmDir,
                            const sead::Vector3f& rForce) {
    sead::Vector3f target = rArmDir + rForce;
    target *= 1.0f / nerd::sqrt(target.squaredLength());

    sead::Matrix34f rotateMtx;
    rotateMtx.makeIdentity();
    sead::Quatf rotate;
    if (rotate.makeVectorRotation(rArmDir, target)) {
        rotateMtx.fromQuat(rotate);
    }

    sead::Vector3f axisX;
    sead::Vector3f axisY;
    sead::Vector3f axisZ;
    rPose.mMtx.getBase(axisX, 0);
    rPose.mMtx.getBase(axisY, 1);
    rPose.mMtx.getBase(axisZ, 2);

    sead::Vector3f diffX = rotateMtx * axisX - axisX;
    diffX *= 1.0f / (axisX.dot(diffX) + 1.0f);
    mRot[0].add(0, axisY.dot(diffX));
    mRot[0].add(1, axisZ.dot(diffX));

    sead::Vector3f diffY = rotateMtx * axisY - axisY;
    diffY *= 1.0f / (axisY.dot(diffY) + 1.0f);
    mRot[1].add(0, axisZ.dot(diffY));
    mRot[1].add(1, axisX.dot(diffY));

    sead::Vector3f diffZ = rotateMtx * axisZ - axisZ;
    diffZ *= 1.0f / (axisZ.dot(diffZ) + 1.0f);
    mRot[2].add(0, axisX.dot(diffZ));
    mRot[2].add(1, axisY.dot(diffZ));
}
