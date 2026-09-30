#include "Library/Play/Camera/CameraVerticalAbsorber.hpp"

#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraPoser_RS.hpp"
#include "Library/Camera/CameraStartInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(CameraVerticalAbsorber, FollowGround)

class CameraVerticalAbsorberNrvFollowAbsolute : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<CameraVerticalAbsorber>()->exeFollowAbsolute();
    }
};

class CameraVerticalAbsorberNrvFollowClimbPoleNoInterp : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<CameraVerticalAbsorber>()->exeFollowClimbPole();
    }
};

class CameraVerticalAbsorberNrvFollowSlow : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<CameraVerticalAbsorber>()->exeFollow();
    }
};

NERVE_DECL(CameraVerticalAbsorber, Absorb)
NERVE_DECL(CameraVerticalAbsorber, Follow)
NERVE_DECL(CameraVerticalAbsorber, FollowClimbPole)

NERVES_MAKE_NOSTRUCT(CameraVerticalAbsorber, FollowGround, FollowAbsolute, FollowClimbPoleNoInterp,
                     FollowSlow, Absorb, Follow, FollowClimbPole)

inline void updateAbsorbVec(sead::Vector3f* pAbsorbVec, const CameraPoser_RS* pPoser,
                            const sead::Vector3f& rPrevTrans) {
    sead::Vector3f gravity = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetGravity(&gravity, pPoser);
    *pAbsorbVec = pPoser->getAt() - rPrevTrans;
    parallelizeVec(pAbsorbVec, gravity, *pAbsorbVec);
}

const f32 sAbsorbStartScreenPosUp = 50.0f;
const f32 sAbsorbStartScreenPosUpMoonGravity = 100.0f;

inline f32 calcFollowRate(const CameraVerticalAbsorber* pAbsorber,
                          const CameraPoser_RS* pPoser) {
    if (alCameraPoserFunction::isPlayerTypeHighJump(pPoser) ||
        isNerve(pAbsorber, &NrvCameraVerticalAbsorberFollowSlow)) {
        return 0.02f;
    }

    if (isNerve(pAbsorber, &NrvCameraVerticalAbsorberFollowClimbPoleNoInterp)) {
        return 0.3f;
    }

    if (isNerve(pAbsorber, &NrvCameraVerticalAbsorberFollowClimbPole)) {
        return calcNerveValue(pAbsorber, 60, 0.05f, 0.3f);
    }

    if (pAbsorber->getFollowRate()) {
        return *pAbsorber->getFollowRate();
    }

    return 0.05f;
}

void updateLerpRate(f32* pRate, const CameraVerticalAbsorber* pAbsorber,
                    const CameraPoser_RS* pPoser, f32 rateA, f32 rateB) {
    f32 prev = *pRate;
    f32 followRate = calcFollowRate(pAbsorber, pPoser);
    *pRate = lerpValue(rateB, *pRate, lerpValue(rateA, prev, followRate));
}

}  // namespace

namespace al {

/**
 * Creates the vertical absorber of a camera.
 * @param pPoser Camera poser.
 * @param isNoCameraPosAbsorb Whether only the look at position is absorbed.
 */
CameraVerticalAbsorber::CameraVerticalAbsorber(const CameraPoser_RS* pPoser,
                                               bool isNoCameraPosAbsorb)
    : NerveExecutor("カメラの縦パン"), mCameraPoser(pPoser),
      mIsNoCameraPosAbsorb(isNoCameraPosAbsorb) {
    initNerve(&NrvCameraVerticalAbsorberFollowGround, 0);
}

/**
 * Loads the absorb parameters.
 * @param rIter Camera parameter iterator.
 */
void CameraVerticalAbsorber::load(const ByamlIter& rIter) {
    ByamlIter iter;

    if (!rIter.tryGetIterByKey(&iter, "VerticalAbsorb")) {
        return;
    }

    tryGetByamlF32(&mAbsorbScreenPosUp, iter, "AbsorbScreenPosUp");
    tryGetByamlF32(&mAbsorbScreenPosDown, iter, "AbsorbScreenPosDown");
    tryGetByamlF32(&mHighJumpJudgeSpeedV, iter, "HighJumpJudgeSpeedV");

    ByamlIter advanceIter;

    if (iter.tryGetIterByKey(&advanceIter, "AdvanceAbsorbUp")) {
        mIsAdvanceAbsorbUp = true;
        mAdvanceAbsorbScreenPosUp = getByamlKeyFloat(advanceIter, "AdvanceAbsorbScreenPosUp");
    }
}

/**
 * Starts absorbing the vertical movement of the target.
 * @param rPos Look at position of the camera.
 * @param rInfo Camera start info.
 */
void CameraVerticalAbsorber::start(const sead::Vector3f& rPos, const CameraStartInfo& rInfo) {
    alCameraPoserFunction::calcTargetFront(&mPrevTargetFront, mCameraPoser);
    mAbsorbVec = {0.0f, 0.0f, 0.0f};
    mPrevTargetTrans = rPos;

    if (!isValid() || alCameraPoserFunction::isPlayerTypeNotTouchGround(mCameraPoser)) {
        return setNerve(this, &NrvCameraVerticalAbsorberFollowAbsolute);
    }

    if (alCameraPoserFunction::isTargetClimbPole(mCameraPoser)) {
        return setNerve(this, &NrvCameraVerticalAbsorberFollowClimbPoleNoInterp);
    }

    if (alCameraPoserFunction::isTargetGrabCeil(mCameraPoser)) {
        return setNerve(this, &NrvCameraVerticalAbsorberFollowSlow);
    }

    if (!rInfo._25 || alCameraPoserFunction::isTargetCollideGround(mCameraPoser)) {
        return setNerve(this, &NrvCameraVerticalAbsorberFollowGround);
    }

    mPrevTargetTrans = alCameraPoserFunction::getPreLookAtPos(mCameraPoser);
    const CameraPoser_RS* poser = mCameraPoser;
    sead::Vector3f gravity = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetGravity(&gravity, poser);
    mAbsorbVec = rPos - mPrevTargetTrans;
    parallelizeVec(&mAbsorbVec, gravity, mAbsorbVec);
    setNerve(this, &NrvCameraVerticalAbsorberAbsorb);
}

/**
 * Returns whether the absorber is valid.
 * @return Whether the absorber is valid.
 */
bool CameraVerticalAbsorber::isValid() const {
    return !_1aa && !mIsInvalidated;
}

/**
 * Updates the absorbed vertical offset.
 */
void CameraVerticalAbsorber::update() {
    if (mIsStopUpdate) {
        return;
    }

    updateAbsorbVec(&mAbsorbVec, mCameraPoser, mPrevTargetTrans);

    mLookAtCamera.setPos(mCameraPoser->getEye());
    mLookAtCamera.setAt(mCameraPoser->getAt());
    mLookAtCamera.setUp(mCameraPoser->getUp());
    mLookAtCamera.normalizeUp();
    makeLookAtCamera(&mLookAtCamera);
    mLookAtCamera.updateViewMatrix();
    mProjection.set(alCameraPoserFunction::getNear(mCameraPoser),
                    alCameraPoserFunction::getFar(mCameraPoser),
                    sead::Mathf::deg2rad(mCameraPoser->getFovyDegree()),
                    alCameraPoserFunction::getAspect(mCameraPoser));
    alCameraPoserFunction::calcTargetFront(&mTargetFront, mCameraPoser);

    if (!isNerve(this, &NrvCameraVerticalAbsorberFollowGround) &&
        alCameraPoserFunction::isTargetCollideGround(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowGround);
    }

    if (!isNerve(this, &NrvCameraVerticalAbsorberFollowAbsolute) &&
        alCameraPoserFunction::isPlayerTypeNotTouchGround(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowAbsolute);
    }

    updateNerve();

    sead::Vector3f offset = {0.0f, 0.0f, 0.0f};

    if (mIsKeepInFrame) {
        sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetTransWithOffset(&targetTrans, mCameraPoser);
        alCameraPoserFunction::calcOffsetCameraKeepInFrameV(
            &offset, &mLookAtCamera, targetTrans, mCameraPoser, mKeepInFrameOffsetUp,
            alCameraPoserFunction::isPlayerTypeHighJump(mCameraPoser) ? 300.0f :
                                                                        mKeepInFrameOffsetDown);
        mAbsorbVec -= offset;
    }

    mPrevTargetTrans.set(mCameraPoser->getAt() - mAbsorbVec);
    mPrevTargetFront = mTargetFront;
}

/**
 * Applies the absorbed offset to a camera pose.
 * @param pCamera Camera pose.
 */
void CameraVerticalAbsorber::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    if (!isValid()) {
        return;
    }

    pCamera->setAt(pCamera->getAt() - mAbsorbVec);

    if (!mIsNoCameraPosAbsorb) {
        pCamera->setPos(pCamera->getPos() - mAbsorbVec);
    }
}

/**
 * Stops absorbing and follows the target.
 */
void CameraVerticalAbsorber::liberateAbsorb() {
    if (isNerve(this, &NrvCameraVerticalAbsorberAbsorb)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
    }
}

/**
 * Keeps the camera height while the target jumps.
 */
void CameraVerticalAbsorber::exeAbsorb() {
    if (isFirstStep(this)) {
        mIsExistCollisionUnderTarget = false;
        mLerp2 = 0.0f;
        mLerp1 = 0.0f;
    }

    if (isGreaterEqualStep(this, 3) && calcAngleDegree(mTargetFront, mPrevTargetFront) > 170.0f &&
        alCameraPoserFunction::calcTargetSpeedV(mCameraPoser) > 18.0f) {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
        return;
    }

    if (!alCameraPoserFunction::isTargetCollideGround(mCameraPoser)) {
        f32 speedV = alCameraPoserFunction::calcTargetSpeedV(mCameraPoser);

        if (mHighJumpJudgeSpeedV < speedV) {
            setNerve(this, &NrvCameraVerticalAbsorberFollow);
            return;
        }
    }

    if (alCameraPoserFunction::isTargetClimbPole(mCameraPoser) &&
        !mCameraPoser->isCalcEndAfterInterpole()) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowClimbPole);
        return;
    }

    if (alCameraPoserFunction::isTargetGrabCeil(mCameraPoser) ||
        alCameraPoserFunction::isTargetWallCatch(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowSlow);
        return;
    }

    sead::Vector2f screenPos = {0.0f, 0.0f};
    {
        const sead::Vector3f& at = mCameraPoser->getAt();
        sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                                static_cast<u32>(getDisplayHeight()));
        mLookAtCamera.projectByMatrix(&screenPos, at, mProjection, viewport);
    }

    screenPos.x += static_cast<u32>(getDisplayWidth()) * 0.5f;
    screenPos.y = static_cast<u32>(getDisplayHeight()) * 0.5f - screenPos.y;

    if (mAbsorbScreenPosDown < screenPos.y || screenPos.y < mAbsorbScreenPosUp) {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
        return;
    }

    sead::Vector3f absorbV = {0.0f, 0.0f, 0.0f};
    sead::Vector3f dir = mLookAtCamera.getAt() - mLookAtCamera.getPos();
    normalize(&dir);
    parallelizeVec(&absorbV, dir, mAbsorbVec);

    if (absorbV.length() > 1000.0f ||
        alCameraPoserFunction::isExistWallCollisionUnderTarget(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
        return;
    }

    if (alCameraPoserFunction::isExistCollisionUnderTarget(mCameraPoser)) {
        if (!mIsExistCollisionUnderTarget) {
            mUnderTargetCollisionPos =
                alCameraPoserFunction::getUnderTargetCollisionPos(mCameraPoser);
            mUnderTargetCollisionNormal =
                alCameraPoserFunction::getUnderTargetCollisionNormal(mCameraPoser);
            mIsExistCollisionUnderTarget = true;
        } else {
            sead::Vector3f gravity = {0.0f, 0.0f, 0.0f};
            alCameraPoserFunction::calcTargetGravity(&gravity, mCameraPoser);
            sead::Vector3f diff =
                alCameraPoserFunction::getUnderTargetCollisionPos(mCameraPoser) -
                mUnderTargetCollisionPos;
            parallelizeVec(&diff, gravity, diff);

            if (isNearZero(diff, 0.001f) || !(diff.dot(gravity) < 0.0f)) {
                mIsExistCollisionUnderTarget = false;
            } else {
                if (!(diff.length() < 30.0f)) {
                    setNerve(this, &NrvCameraVerticalAbsorberFollowSlow);
                    return;
                }

                sead::Vector3f diffNew =
                    alCameraPoserFunction::getUnderTargetCollisionPos(mCameraPoser) -
                    mUnderTargetCollisionPos;
                sead::Vector3f diffPrev = diffNew;
                parallelizeVec(&diffNew,
                               alCameraPoserFunction::getUnderTargetCollisionNormal(mCameraPoser),
                               diffNew);
                parallelizeVec(&diffPrev, mUnderTargetCollisionNormal, diffPrev);
                f32 lengthNew = diffNew.length();
                f32 lengthPrev = diffPrev.length();

                if (!((lengthNew < lengthPrev ? lengthNew : lengthPrev) < 5.0f)) {
                    setNerve(this, &NrvCameraVerticalAbsorberFollowSlow);
                    return;
                }

                mLerp2 = diff.length();
                mUnderTargetCollisionPos =
                    alCameraPoserFunction::getUnderTargetCollisionPos(mCameraPoser);
            }
        }

        f32 length = mAbsorbVec.length();
    f32 prevAbsorbLength = mLerp2;
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, 0.0f);
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, mLerp2);
    f32 rate = normalize(prevAbsorbLength - mLerp2, 0.0f, mAbsorbVec.length());
    f32 curLength = mAbsorbVec.length();

    if (curLength > 0.0f) {
        mAbsorbVec *= length * (1.0f - rate) / curLength;
    }
    } else {
        mIsExistCollisionUnderTarget = false;
    }

    const f32& startScreenPosUp =
        mIsAdvanceAbsorbUp ? mAdvanceAbsorbScreenPosUp :
        alCameraPoserFunction::isTargetInMoonGravity(mCameraPoser) ?
                             sAbsorbStartScreenPosUpMoonGravity :
                             sAbsorbStartScreenPosUp;
    if (mAbsorbScreenPosUp < startScreenPosUp && screenPos.y < startScreenPosUp) {
        f32 rate = easeIn(normalize(screenPos.y, mAbsorbScreenPosUp, startScreenPosUp));
        mLerp1 = lerpValue(mLerp1, lerpValue(mLerp1, rate, 0.05f), 0.05f);
    }

    mAbsorbVec *= 1.0f - mLerp1;
}

/**
 * Follows the target with a rate depending on the state.
 */
void CameraVerticalAbsorber::exeFollow() {
    if (isFirstStep(this)) {
        mLerp1 = calcFollowRate(this, mCameraPoser);
    }

    updateLerpRate(&mLerp1, this, mCameraPoser, 0.05f, 0.05f);
    f32 length = mAbsorbVec.length();
    f32 prevAbsorbLength = mLerp2;
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, 0.0f);
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, mLerp2);
    f32 rate = normalize(prevAbsorbLength - mLerp2, 0.0f, mAbsorbVec.length());
    f32 curLength = mAbsorbVec.length();

    if (curLength > 0.0f) {
        mAbsorbVec *= length * (1.0f - rate) / curLength;
    }

    mAbsorbVec *= 1.0f - mLerp1;
}

/**
 * Follows the target on the ground.
 */
void CameraVerticalAbsorber::exeFollowGround() {
    updateLerpRate(&mLerp1, this, mCameraPoser, 0.2f, 0.75f);
    f32 length = mAbsorbVec.length();
    f32 prevAbsorbLength = mLerp2;
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, 0.0f);
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, mLerp2);
    f32 rate = normalize(prevAbsorbLength - mLerp2, 0.0f, mAbsorbVec.length());
    f32 curLength = mAbsorbVec.length();

    if (curLength > 0.0f) {
        mAbsorbVec *= length * (1.0f - rate) / curLength;
    }

    mAbsorbVec *= 1.0f - mLerp1;

    if (isGreaterEqualStep(this, 3) &&
        !alCameraPoserFunction::isTargetCollideGround(mCameraPoser)) {
        const CameraPoser_RS* poser = mCameraPoser;

        if (alCameraPoserFunction::isExistSlopeCollisionUnderTarget(poser) &&
            !(alCameraPoserFunction::calcTargetSpeedH(poser) < 10.0f) &&
            !(alCameraPoserFunction::getUnderTargetCollisionNormal(poser).y < 0.342f)) {
            sead::Vector3f dir = mLookAtCamera.getPos() - mLookAtCamera.getAt();

            if (tryNormalizeOrZero(&dir) && dir.y < 0.2588f) {
                setNerve(this, &NrvCameraVerticalAbsorberFollow);
                return;
            }
        }

        setNerve(this, &NrvCameraVerticalAbsorberAbsorb);
    }
}

/**
 * Follows the target while it climbs a pole.
 */
void CameraVerticalAbsorber::exeFollowClimbPole() {
    updateLerpRate(&mLerp1, this, mCameraPoser, 0.2f, 0.75f);
    f32 length = mAbsorbVec.length();
    f32 prevAbsorbLength = mLerp2;
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, 0.0f);
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, mLerp2);
    f32 rate = normalize(prevAbsorbLength - mLerp2, 0.0f, mAbsorbVec.length());
    f32 curLength = mAbsorbVec.length();

    if (curLength > 0.0f) {
        mAbsorbVec *= length * (1.0f - rate) / curLength;
    }

    mAbsorbVec *= 1.0f - mLerp1;

    if (!alCameraPoserFunction::isTargetClimbPole(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
    }
}

/**
 * Keeps the target position without absorbing.
 */
void CameraVerticalAbsorber::exeFollowAbsolute() {
    mAbsorbVec *= 0.8f;
}

/**
 * Follows the target while it swims.
 */
void CameraVerticalAbsorber::exeFollowWater() {
    updateLerpRate(&mLerp1, this, mCameraPoser, 0.2f, 0.75f);
    f32 length = mAbsorbVec.length();
    f32 prevAbsorbLength = mLerp2;
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, 0.0f);
    mLerp2 = lerpValue(0.9f, prevAbsorbLength, mLerp2);
    f32 rate = normalize(prevAbsorbLength - mLerp2, 0.0f, mAbsorbVec.length());
    f32 curLength = mAbsorbVec.length();

    if (curLength > 0.0f) {
        mAbsorbVec *= length * (1.0f - rate) / curLength;
    }

    mAbsorbVec *= 1.0f - mLerp1;

    if (isGreaterEqualStep(this, 3) && !alCameraPoserFunction::isTargetInWater(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberAbsorb);
    }
}

/**
 * Returns whether the absorber is absorbing the vertical movement.
 * @return Whether the absorber is absorbing.
 */
bool CameraVerticalAbsorber::isAbsorbing() const {
    return isValid() && isNerve(this, &NrvCameraVerticalAbsorberAbsorb);
}

/**
 * Disables the absorber.
 */
void CameraVerticalAbsorber::invalidate() {
    mIsInvalidated = true;

    if (!isNerve(this, &NrvCameraVerticalAbsorberFollowAbsolute)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowAbsolute);
    }
}

/**
 * Resets the absorbed offset if the unabsorbed camera would be inside collision.
 * @param rPos Look at position of the camera.
 */
void CameraVerticalAbsorber::tryResetAbsorbVecIfInCollision(const sead::Vector3f& rPos) {
    if (!alCameraPoserFunction::checkFirstCameraCollisionArrow(nullptr, nullptr, mCameraPoser,
                                                               rPos + mAbsorbVec, -mAbsorbVec)) {
        return;
    }

    mAbsorbVec = {0.0f, 0.0f, 0.0f};

    if (alCameraPoserFunction::isTargetCollideGround(mCameraPoser)) {
        setNerve(this, &NrvCameraVerticalAbsorberFollowGround);
    } else {
        setNerve(this, &NrvCameraVerticalAbsorberFollow);
    }
}

}  // namespace al
