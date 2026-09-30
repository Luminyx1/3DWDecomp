#include "Library/Collision/SphereInterpolator.hpp"

namespace al {
/**
 * Starts interpolating a sphere between two positions.
 * @param rPosStart start position
 * @param rPosEnd end position
 * @param sizeStart radius at the start
 * @param sizeEnd radius at the end
 * @param steps step length
 */
void SphereInterpolator::startInterp(const sead::Vector3f& rPosStart,
                                     const sead::Vector3f& rPosEnd, f32 sizeStart, f32 sizeEnd,
                                     f32 steps) {
    mCurrentStep = 0.0f;
    mPrevStep = 0.0f;
    mPos = rPosStart;
    mMove = rPosEnd - rPosStart;
    mSizeStart = sizeStart;
    mSizeEnd = sizeEnd;

    f32 dist = mMove.length() + sizeEnd - sizeStart;
    mStepSize = (dist <= 0.0f) ? 1.0f : steps / dist;
}

/**
 * Advances the interpolation by one step.
 */
void SphereInterpolator::nextStep() {
    s32 curStep = *reinterpret_cast<s32*>(&mCurrentStep);
    f32 stepAsFloat = *reinterpret_cast<f32*>(&curStep);
    f32 newStep = sead::Mathf::clampMax(stepAsFloat + mStepSize, 1.0f);
    *reinterpret_cast<s32*>(&mPrevStep) = curStep;
    mCurrentStep = newStep;
}

/**
 * Calculates the current interpolated position.
 * @param pPos output position
 */
void SphereInterpolator::calcInterpPos(sead::Vector3f* pPos) const {
    f32 step = mCurrentStep;
    pPos->x = mMove.x * step + mPos.x;
    pPos->y = mMove.y * step + mPos.y;
    pPos->z = mMove.z * step + mPos.z;
}

/**
 * Calculates the current interpolated position, radius and remaining movement.
 * @param pPos output position
 * @param pSize output radius
 * @param pRemainMoveVec output remaining movement, or nullptr
 */
void SphereInterpolator::calcInterp(sead::Vector3f* pPos, f32* pSize,
                                    sead::Vector3f* pRemainMoveVec) const {
    calcInterpPos(pPos);
    *pSize = mSizeStart + (mSizeEnd - mSizeStart) * mCurrentStep;
    calcRemainMoveVector(pRemainMoveVec);
}

/**
 * Calculates the movement remaining after the current step.
 * @param pRemainMoveVec output remaining movement, or nullptr
 */
void SphereInterpolator::calcRemainMoveVector(sead::Vector3f* pRemainMoveVec) const {
    if (pRemainMoveVec) {
        f32 remainStep = 1.0f - mCurrentStep;
        pRemainMoveVec->x = mMove.x * remainStep;
        pRemainMoveVec->y = mMove.y * remainStep;
        pRemainMoveVec->z = mMove.z * remainStep;
    }
}

/**
 * Calculates the movement up to the current step.
 * @param pMoveVec output movement
 */
void SphereInterpolator::getMoveVector(sead::Vector3f* pMoveVec) {
    f32 step = mCurrentStep;
    pMoveVec->x = mMove.x * step;
    pMoveVec->y = mMove.y * step;
    pMoveVec->z = mMove.z * step;
}
}  // namespace al
