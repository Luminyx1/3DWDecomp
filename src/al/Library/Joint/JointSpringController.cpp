#include "Library/Joint/JointSpringController.hpp"

#include <math/seadQuat.h>

#include "Library/Joint/JointControllerKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"

namespace al {
namespace {
const sead::Vector3f cDefaultChildLocalPos(200.0f, 0.0f, 0.0f);
}  // namespace

JointSpringController::JointSpringController() : mChildLocalPos(cDefaultChildLocalPos) {}

/**
 * Sets the local position of the child point.
 * @param rPos Local child position.
 */
void JointSpringController::setChildLocalPos(const sead::Vector3f& rPos) {
    mChildLocalPos.set(rPos);
}

/**
 * Sets a matrix whose translation is used as the child position.
 * @param pMtx Child local matrix.
 */
void JointSpringController::setChildLocalMtxPtr(const sead::Matrix34f* pMtx) {
    mChildLocalMtxPtr = pMtx;
}

/**
 * Sets the spring stiffness.
 * @param stability Spring stiffness.
 */
void JointSpringController::setStability(f32 stability) {
    mStability = stability;
}

/**
 * Sets the velocity damping factor.
 * @param friction Damping factor.
 */
void JointSpringController::setFriction(f32 friction) {
    mFriction = friction;
}

/**
 * Sets the maximum bend angle.
 * @param degree Limit angle in degrees.
 */
void JointSpringController::setLimitDegree(f32 degree) {
    mLimitDegree = degree;
}

/**
 * Sets the blend rate.
 * @param rate Control rate.
 */
void JointSpringController::setControlRate(f32 rate) {
    mControlRate = rate;
}

/**
 * Increases the blend rate, clamped to [0, 1].
 * @param rate Amount to add.
 */
void JointSpringController::addControlRate(f32 rate) {
    f32 newRate = mControlRate + rate;
    if (newRate < 0.0f) {
        newRate = 0.0f;
    } else if (newRate > 1.0f) {
        newRate = 1.0f;
    }

    mControlRate = newRate;
}

/**
 * Decreases the blend rate, clamped to [0, 1].
 * @param rate Amount to subtract.
 */
void JointSpringController::subControlRate(f32 rate) {
    f32 newRate = mControlRate - rate;
    if (newRate < 0.0f) {
        newRate = 0.0f;
    } else if (newRate > 1.0f) {
        newRate = 1.0f;
    }

    mControlRate = newRate;
}

/**
 * Clears the velocity and reinitializes on the next update.
 */
void JointSpringController::reset() {
    mVelocity.set(sead::Vector3f::zero);
    mIsInitialized = false;
}

/**
 * Calculates the world position of the child point.
 * @param pPos Receives the child position.
 * @param pMtx Joint matrix.
 */
void JointSpringController::calcChildPos(sead::Vector3f* pPos, const sead::Matrix34f* pMtx) const {
    sead::Vector3f localPos;
    if (mChildLocalMtxPtr) {
        mChildLocalMtxPtr->getTranslation(localPos);
    } else {
        localPos = mChildLocalPos;
    }

    pPos->setMul(*pMtx, localPos);
}

void JointSpringController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    bool isPaused = isPausedJointControllers();

    if (!mIsInitialized) {
        pMtx->getTranslation(mChildPos);
        mIsInitialized = true;
        return;
    }

    if (isNearZero(mControlRate)) {
        pMtx->getTranslation(mChildPos);
        if (!isPaused) {
            mVelocity *= mFriction;
        }

        return;
    }

    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    calcMtxScale(&scale, *pMtx);
    if (isNearZero(scale.x) || isNearZero(scale.y) || isNearZero(scale.z)) {
        return;
    }

    sead::Vector3f childPos;
    calcChildPos(&childPos, pMtx);
    sead::Vector3f trans;
    pMtx->getTranslation(trans);

    sead::Vector3f velocity;
    if (isPaused) {
        velocity = mVelocity;
    } else {
        mVelocity = ((childPos - mChildPos) * mStability + mVelocity) * mFriction;
        velocity = mVelocity;
    }

    sead::Vector3f nextDir = mChildPos + velocity - trans;
    if (normalizeOrZero(&nextDir)) {
        return;
    }

    sead::Vector3f currentDir = childPos - trans;
    if (normalizeOrZero(&currentDir)) {
        return;
    }

    sead::Vector3f invScale = {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z};
    preScaleMtx(pMtx, invScale);
    sead::Quatf quat;
    pMtx->toQuat(quat);
    turnQuat(&quat, quat, currentDir, nextDir,
             sead::Mathf::deg2rad(mLimitDegree * mControlRate));
    pMtx->makeQT(quat, trans);
    preScaleMtx(pMtx, scale);

    if (!isPaused) {
        calcChildPos(&mChildPos, pMtx);
    }
}

}  // namespace al
