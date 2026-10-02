#include "Library/Play/Camera/CameraVerticalAbsorber2DGalaxy.hpp"

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(CameraVerticalAbsorber2DGalaxy, None)
NERVE_DECL(CameraVerticalAbsorber2DGalaxy, Ground)
NERVE_DECL(CameraVerticalAbsorber2DGalaxy, Limit)
NERVE_DECL(CameraVerticalAbsorber2DGalaxy, LimitAfter)
NERVE_DECL(CameraVerticalAbsorber2DGalaxy, LimitOver)

NERVES_MAKE_NOSTRUCT(CameraVerticalAbsorber2DGalaxy, None, Ground, Limit, LimitAfter, LimitOver)

}  // namespace

namespace al {

/**
 * Creates the vertical absorber used on 2D planet gravity.
 */
CameraVerticalAbsorber2DGalaxy::CameraVerticalAbsorber2DGalaxy()
    : NerveExecutor("2D惑星重力環境での縦パン") {
    initNerve(&NrvCameraVerticalAbsorber2DGalaxyNone, 0);
}

/**
 * Updates the cached target transform, gravity, up direction and ground state.
 * @param pPoser Camera poser.
 */
inline void CameraVerticalAbsorber2DGalaxy::updateTargetInfo(const CameraPoser_RS* pPoser) {
    alCameraPoserFunction::calcTargetTrans(&mTargetTrans, pPoser);
    alCameraPoserFunction::calcTargetGravity(&mTargetGravity, pPoser);
    alCameraPoserFunction::calcTargetUp(&mTargetUp, pPoser);
    mIsTargetCollideGround = alCameraPoserFunction::isTargetCollideGround(pPoser);
}

/**
 * Starts the absorber.
 * @param pPoser Camera poser.
 */
void CameraVerticalAbsorber2DGalaxy::start(const CameraPoser_RS* pPoser) {
    updateTargetInfo(pPoser);

    if (mIsTargetCollideGround) {
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyGround);
    }

    setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyNone);
}

/**
 * Updates the target state and the absorbed height.
 * @param pPoser Camera poser.
 */
void CameraVerticalAbsorber2DGalaxy::update(const CameraPoser_RS* pPoser) {
    mPrevTargetTrans.set(mTargetTrans);
    mPrevTargetGravity.set(mTargetGravity);
    updateTargetInfo(pPoser);
    updateNerve();
}

/**
 * Applies the absorbed height to a position.
 * @param pPos Position to modify.
 */
void CameraVerticalAbsorber2DGalaxy::applyLimit(sead::Vector3f* pPos) const {
    *pPos -= mLimitHeight * mLimitDir;
}

/**
 * Releases the absorbed height while the target is in the air.
 */
void CameraVerticalAbsorber2DGalaxy::exeNone() {
    if (mIsTargetCollideGround) {
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyGround);
    }

    mLimitTargetHeight = lerpValue(0.075f, mLimitTargetHeight, 0.0f);
    mLimitHeight = lerpValue(0.05f, mLimitHeight, 0.0f);
}

/**
 * Releases the absorbed height while the target is on the ground.
 */
void CameraVerticalAbsorber2DGalaxy::exeGround() {
    if (mIsTargetCollideGround) {
        mLimitTargetHeight = lerpValue(0.075f, mLimitTargetHeight, 0.0f);
        mLimitHeight = lerpValue(0.05f, mLimitHeight, 0.0f);
        return;
    }

    mLimitDir = -mPrevTargetGravity;
    mLimitHeight = (mTargetTrans - mPrevTargetTrans).dot(mLimitDir);
    setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimit);
}

/**
 * Absorbs the vertical movement of the target while it is in the air.
 */
void CameraVerticalAbsorber2DGalaxy::exeLimit() {
    if (isFirstStep(this)) {
        mLimitStartUp.set(mTargetUp);
    }

    mLimitHeight += (-mPrevTargetTrans + mTargetTrans).dot(mLimitDir);

    if (mIsTargetCollideGround) {
        mLimitTargetHeight = mLimitHeight;
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimitAfter);
    }

    if (mLimitHeight >= 600.0f) {
        mLimitTargetHeight = mLimitHeight;
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimitOver);
    }

    if (calcAngleDegree(mLimitStartUp, mTargetUp) > 30.0f) {
        mLimitTargetHeight = mLimitHeight;
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimitOver);
    }
}

/**
 * Follows the target after it moved too far away from the absorbed height.
 */
void CameraVerticalAbsorber2DGalaxy::exeLimitOver() {
    mLimitTargetHeight = lerpValue(0.1f, mLimitTargetHeight, 0.0f);
    mLimitHeight = lerpValue(0.05f, mLimitHeight, mLimitTargetHeight);

    if (mIsTargetCollideGround) {
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimitAfter);
    }
}

/**
 * Releases the absorbed height after the target landed.
 */
void CameraVerticalAbsorber2DGalaxy::exeLimitAfter() {
    mLimitTargetHeight = lerpValue(0.075f, mLimitTargetHeight, 0.0f);
    mLimitHeight = lerpValue(0.05f, mLimitHeight, mLimitTargetHeight);

    if (!mIsTargetCollideGround) {
        return setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyLimit);
    }

    if (isNearZero(mLimitHeight, 0.01f)) {
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxyGround);
    }
}

}  // namespace al
