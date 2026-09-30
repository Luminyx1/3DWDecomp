#include "Library/Camera/CameraParamMoveLimit.hpp"

#include "Library/Camera/CameraPoser_RS.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates a move limit in the view space of a poser.
 * @param pPoser Poser whose view matrix defines the limit space.
 * @return New move limit.
 */
CameraParamMoveLimit* CameraParamMoveLimit::create(const CameraPoser_RS* pPoser) {
    CameraParamMoveLimit* moveLimit = new CameraParamMoveLimit();
    moveLimit->mViewMtx = pPoser->getViewMtx();
    moveLimit->mInvViewMtx.setInverse(pPoser->getViewMtx());
    return moveLimit;
}

/**
 * Resets the pause state and loads the limits from the "MoveLimit" block.
 * @param rIter Camera parameter iterator.
 */
void CameraParamMoveLimit::load(const ByamlIter& rIter) {
    ByamlIter moveLimitIter;
    mIsPauseApply = false;
    mIsPauseInterpolate = false;
    mHasWaterHeight = false;
    mWaterHeight = 0.0f;
    mPauseOffset.set(sead::Vector3f::zero);
    mPauseOffsetTarget.set(sead::Vector3f::zero);
    if (!rIter.tryGetIterByKey(&moveLimitIter, "MoveLimit")) {
        return;
    }

    tryGetByamlF32(&mRotYDegree, moveLimitIter, "BaseAngleH");
    if (tryGetByamlF32(&mPlus.x, moveLimitIter, "PlusX")) {
        mHasPlusX = true;
    }
    if (tryGetByamlF32(&mMinus.x, moveLimitIter, "MinusX")) {
        mHasMinusX = true;
    }
    if (tryGetByamlF32(&mPlus.y, moveLimitIter, "PlusY")) {
        mHasPlusY = true;
    }
    if (tryGetByamlF32(&mMinus.y, moveLimitIter, "MinusY")) {
        mHasMinusY = true;
    }
    if (tryGetByamlF32(&mPlus.z, moveLimitIter, "PlusZ")) {
        mHasPlusZ = true;
    }
    if (tryGetByamlF32(&mMinus.z, moveLimitIter, "MinusZ")) {
        mHasMinusZ = true;
    }
}

/**
 * Enables or disables clamping, starting an interpolation when the state changes.
 * @param isPause Whether clamping is paused.
 */
void CameraParamMoveLimit::setPauseApply(bool isPause) {
    if (isPause != mIsPauseApply) {
        mIsPauseInterpolate = true;
        mPauseOffset.set(sead::Vector3f::zero);
    }
    mIsPauseApply = isPause;
}

/**
 * Sets a height offset added to the vertical limits.
 * @param height Height offset.
 */
void CameraParamMoveLimit::setWaterHeight(f32 height) {
    mWaterHeight = height;
    mHasWaterHeight = true;
}

/**
 * Moves the camera towards the limited look-at position.
 * @param pCamera Camera to move.
 * @param rPos Limited look-at position in limit space.
 */
void CameraParamMoveLimit::pauseInterpolate(sead::LookAtCamera* pCamera, sead::Vector3f& rPos) {
    sead::Vector3f at;
    {
        sead::Vector3f pos = rPos;
        f32 rotYDegree = mRotYDegree;
        sead::Matrix34f mtxRotateY = sead::Matrix34f::ident;
        rotateMtxYDirDegree(&mtxRotateY, mtxRotateY, rotYDegree);
        sead::Vector3f localAt;
        localAt.setMul(mtxRotateY, pos);
        at.setMul(mViewMtx, localAt);
    }
    mPauseOffsetTarget = at - pCamera->getAt();
    lerpVec(&mPauseOffset, mPauseOffset, mPauseOffsetTarget, 0.02f);
    if (isNearZero(mPauseOffset - mPauseOffsetTarget, 0.001f)) {
        mIsPauseInterpolate = false;
        mPauseOffset.set(mPauseOffsetTarget);
    }
    pCamera->setAt(pCamera->getAt() + mPauseOffset);
    pCamera->setPos(pCamera->getPos() + mPauseOffset);
}

void CameraParamMoveLimit::apply(sead::LookAtCamera* pCamera) {
    if (mIsPauseApply) {
        sead::Vector3f viewAt = pCamera->getAt();
        viewAt.setMul(mInvViewMtx, viewAt);
        sead::Matrix34f mtxRotateY = sead::Matrix34f::ident;
        rotateMtxYDirDegree(&mtxRotateY, mtxRotateY, -mRotYDegree);
        viewAt.setMul(mtxRotateY, viewAt);
        if (mIsPauseInterpolate) {
            pauseInterpolate(pCamera, viewAt);
            return;
        }
        sead::Matrix34f mtxRotateYInv = sead::Matrix34f::ident;
        rotateMtxYDirDegree(&mtxRotateYInv, mtxRotateYInv, mRotYDegree);
        sead::Vector3f rotatedAt;
        rotatedAt.setMul(mtxRotateYInv, viewAt);
        sead::Vector3f finalAt;
        finalAt.setMul(mViewMtx, rotatedAt);
        mPauseOffset = finalAt - pCamera->getAt();
        pCamera->setAt(pCamera->getAt() + mPauseOffset);
        pCamera->setPos(pCamera->getPos() + mPauseOffset);
    } else {
        sead::Vector3f viewAt = pCamera->getAt();
        viewAt.setMul(mInvViewMtx, viewAt);
        sead::Matrix34f mtxRotateY = sead::Matrix34f::ident;
        rotateMtxYDirDegree(&mtxRotateY, mtxRotateY, -mRotYDegree);
        viewAt.setMul(mtxRotateY, viewAt);
        if (mHasWaterHeight) {
            bool isLimitY = false;
            if (mHasPlusX) {
                viewAt.x = viewAt.x > mPlus.x ? mPlus.x : viewAt.x;
            }
            if (mHasPlusY) {
                viewAt.y = viewAt.y > mPlus.y ? mPlus.y : viewAt.y;
                isLimitY = true;
            }
            if (mHasPlusZ) {
                viewAt.z = viewAt.z > mPlus.z ? mPlus.z : viewAt.z;
            }
            if (mHasMinusX) {
                viewAt.x = viewAt.x < mMinus.x ? mMinus.x : viewAt.x;
            }
            if (mHasMinusY) {
                viewAt.y = viewAt.y < mMinus.y ? mMinus.y : viewAt.y;
                isLimitY = true;
            }
            if (mHasMinusZ) {
                viewAt.z = viewAt.z < mMinus.z ? mMinus.z : viewAt.z;
            }
            if (isLimitY) {
                viewAt.y += mWaterHeight;
            }
        } else {
        if (mHasPlusX) {
            viewAt.x = viewAt.x > mPlus.x ? mPlus.x : viewAt.x;
        }
        if (mHasPlusY) {
            viewAt.y = viewAt.y > mPlus.y ? mPlus.y : viewAt.y;
        }
        if (mHasPlusZ) {
            viewAt.z = viewAt.z > mPlus.z ? mPlus.z : viewAt.z;
        }
        if (mHasMinusX) {
            viewAt.x = viewAt.x < mMinus.x ? mMinus.x : viewAt.x;
        }
        if (mHasMinusY) {
            viewAt.y = viewAt.y < mMinus.y ? mMinus.y : viewAt.y;
        }
        if (mHasMinusZ) {
            viewAt.z = viewAt.z < mMinus.z ? mMinus.z : viewAt.z;
        }
        }
        if (mIsPauseInterpolate) {
            pauseInterpolate(pCamera, viewAt);
            return;
        }
        sead::Matrix34f mtxRotateYInv = sead::Matrix34f::ident;
        rotateMtxYDirDegree(&mtxRotateYInv, mtxRotateYInv, mRotYDegree);
        sead::Vector3f rotatedAt;
        rotatedAt.setMul(mtxRotateYInv, viewAt);
        sead::Vector3f finalAt;
        finalAt.setMul(mViewMtx, rotatedAt);
        mPauseOffset = finalAt - pCamera->getAt();
        pCamera->setAt(pCamera->getAt() + mPauseOffset);
        pCamera->setPos(pCamera->getPos() + mPauseOffset);
    }
}

/**
 * Creates a move limit without limits.
 */
CameraParamMoveLimit::CameraParamMoveLimit() {
    mIsPauseApply = false;
    mIsPauseInterpolate = false;
}

}  // namespace al
