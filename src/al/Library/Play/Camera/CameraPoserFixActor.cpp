#include "Library/Play/Camera/CameraPoserFixActor.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {
using namespace al;

// The nerve keeper of a camera poser holds the poser itself, not its IUseNerve base.
class CameraPoserFixActorNrvFollow : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        static_cast<CameraPoserFixActor*>(static_cast<void*>(pKeeper->getParent<al::IUseNerve>()))
            ->exeFollow();
    }
};

NERVES_MAKE_NOSTRUCT(CameraPoserFixActor, Follow)

/**
 * Places the camera behind the actor, looking at the actor position with an offset.
 * @param pPoser Camera poser to update.
 * @param distance Distance from the look at position.
 * @param pActor Actor to look at.
 * @param rOffset Look at offset in the local space of the actor.
 * @param angleH Horizontal angle relative to the actor front in degrees.
 * @param angleV Vertical angle in degrees.
 * @param isCalcNearestAtFromPreAt Whether to move the look at position towards the previous one.
 * @param rPreLookAtPos Previous look at position.
 */
void calcFixActorCameraPose(CameraPoser_RS* pPoser, f32 distance, const LiveActor* pActor,
                            const sead::Vector3f& rOffset, f32 angleH, f32 angleV,
                            bool isCalcNearestAtFromPreAt, const sead::Vector3f& rPreLookAtPos) {
    if (pActor == nullptr) {
        return;
    }

    sead::Vector3f lookAtPos = pPoser->getAt();
    multVecPose(&lookAtPos, pActor, rOffset);
    pPoser->getAtPtr()->set(lookAtPos);

    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&dir, pActor);
    rotateVectorDegreeY(&dir, angleH);

    f32 radianV = sead::Mathf::deg2rad(angleV);
    f32 lengthH = cosf(radianV);
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= lengthH / length;
    }

    dir.y = sinf(radianV);
    pPoser->setEye(dir * distance + pPoser->getAt());

    if (isCalcNearestAtFromPreAt) {
        sead::Vector3f offset = rPreLookAtPos - pPoser->getEye();
        parallelizeVec(&offset, dir, offset);

        if (!isNearZero(offset) && dir.dot(offset) < 0.0f) {
            pPoser->getAtPtr()->set(offset + pPoser->getEye());
        }
    }
}

/**
 * Places the camera in a fixed direction from the actor, looking at the actor position with an
 * offset.
 * @param pPoser Camera poser to update.
 * @param distance Distance from the look at position.
 * @param pActor Actor to look at.
 * @param rOffset Look at offset in the local space of the actor.
 * @param angleV Vertical angle in degrees.
 * @param pDir Direction from the look at position to the camera. Its height is updated.
 */
inline void calcFixActorCameraPoseDirect(CameraPoser_RS* pPoser, f32 distance,
                                         const LiveActor* pActor, const sead::Vector3f& rOffset,
                                         f32 angleV, sead::Vector3f* pDir) {
    if (pActor == nullptr) {
        return;
    }

    sead::Vector3f lookAtPos = pPoser->getAt();
    multVecPose(&lookAtPos, pActor, rOffset);
    pPoser->getAtPtr()->set(lookAtPos);
    pDir->y = sinf(sead::Mathf::deg2rad(angleV));
    pPoser->setEye(*pDir * distance + pPoser->getAt());
}

/**
 * @param step Current step.
 * @param stepMax Number of steps.
 * @return Progress of the return in the range of 0 to 1.
 */
inline f32 calcReturnRate(s32 step, s32 stepMax) {
    if (stepMax < 2) {
        return 1.0f;
    }

    return static_cast<f32>(step) / static_cast<f32>(stepMax);
}

}  // namespace

namespace al {

/**
 * Creates a camera that looks at an actor from a fixed angle.
 * @param pActor Actor to look at.
 */
CameraPoserFixActor::CameraPoserFixActor(const LiveActor* pActor)
    : CameraPoser_RS("アクター固定"), mTargetActor(pActor), mOffset(0.0f, 0.0f, 0.0f),
      mDistance(1800.0f), mAngleH(0.0f), mAngleV(30.0f), _170(false),
      mIsCalcNearestAtFromPreAt(false), mIsReturn(false), mIsReturnEnd(false),
      mIsDirectAngle(false), mIsReturnLerpDir(false), mStoredCameraPos(sead::Vector3f::zero),
      mStoredLookAtPos(sead::Vector3f::zero), mReturnStepMax(0), mReturnStep(0),
      mReturnOffsetY(0.0f), mDirectAngleDir(sead::Vector3f::zero) {
    mUp.set(sead::Vector3f::ey);
}

/**
 * Creates a camera that looks at an actor from a fixed angle, without a target actor.
 * @param pName Camera name.
 */
CameraPoserFixActor::CameraPoserFixActor(const char* pName)
    : CameraPoser_RS(pName), mTargetActor(nullptr), _150(nullptr), mOffset(0.0f, 0.0f, 0.0f),
      mDistance(1800.0f), mAngleH(0.0f), mAngleV(30.0f), _170(false),
      mIsCalcNearestAtFromPreAt(false), mIsReturn(false), mIsReturnEnd(false),
      mIsDirectAngle(false), mIsReturnToDir(false), mStoredCameraPos(sead::Vector3f::zero),
      mStoredLookAtPos(sead::Vector3f::zero), mReturnStepMax(0), mReturnStep(0),
      mReturnOffsetY(0.0f), mDirectAngleDir(sead::Vector3f::zero) {
    mUp.set(sead::Vector3f::ey);
}

/**
 * Initializes the arrow collider and the nerve.
 */
void CameraPoserFixActor::init() {
    alCameraPoserFunction::initCameraArrowCollider(this);
    initNerve(&NrvCameraPoserFixActorFollow, 0);
}

/**
 * Loads the offset, distance and angles and remembers them as defaults.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFixActor::loadParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&mOffset, rIter, "Offset");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngleH, rIter, "AngleH");
    tryGetByamlF32(&mAngleV, rIter, "AngleV");
    mDefaultOffset = mOffset;
    mDefaultDistance = mDistance;
    mDefaultAngleH = mAngleH;
    mDefaultAngleV = mAngleV;
}

/**
 * Places the camera relative to the target actor.
 * @param rInfo Start info.
 */
void CameraPoserFixActor::start(const CameraStartInfo& rInfo) {
    calcFixActorCameraPose(this, mDistance, mTargetActor, mOffset, mAngleH, mAngleV,
                           mIsCalcNearestAtFromPreAt, mStoredLookAtPos);
}

/**
 * Restores the default offset, distance and angles.
 */
void CameraPoserFixActor::reset() {
    mOffset = mDefaultOffset;
    mDistance = mDefaultDistance;
    mAngleH = mDefaultAngleH;
    mAngleV = mDefaultAngleV;
    mIsReturnEnd = false;
}

/**
 * Stores the camera to return to.
 * @param rCameraPos Camera position.
 * @param rLookAtPos Look at position.
 */
void CameraPoserFixActor::storeCamera(const sead::Vector3f& rCameraPos,
                                      const sead::Vector3f& rLookAtPos) {
    mStoredCameraPos.set(rCameraPos);
    mStoredLookAtPos.set(rLookAtPos);
    mStoredDistance = (mStoredLookAtPos - mStoredCameraPos).length();
    mStoredDir = mStoredCameraPos - mStoredLookAtPos;
    mStoredDir.normalize();
}

/**
 * Uses the current angles as a fixed direction instead of the actor front.
 */
void CameraPoserFixActor::setDirectAngle() {
    mIsDirectAngle = true;
    mDirectAngleDir.x = sinf(sead::Mathf::deg2rad(mAngleH));
    mDirectAngleDir.y = sinf(sead::Mathf::deg2rad(mAngleV));
    mDirectAngleDir.z = cosf(sead::Mathf::deg2rad(mAngleH));
}

/**
 * Uses a fixed direction instead of the actor front.
 * @param rDir Direction from the look at position to the camera.
 */
void CameraPoserFixActor::setDirectAngle(sead::Vector3f& rDir) {
    mIsDirectAngle = true;
    mDirectAngleDir = rDir;
}

/**
 * Everything is updated by the nerve.
 */
void CameraPoserFixActor::update() {}

/**
 * Advances the return, finishing it after the last step.
 */
inline void CameraPoserFixActor::updateReturnStep() {
    if (mReturnStep < mReturnStepMax) {
        mReturnStep++;
        return;
    }

    mIsReturn = false;
    mIsReturnEnd = true;
}

/**
 * Moves the look at position back while the distance approaches the stored one.
 * @param rate Progress of the return.
 */
inline void CameraPoserFixActor::updateReturnLerpDistance(f32 rate) {
    if (rate == 1.0f) {
        mReturnDistance = mStoredDistance;
    } else {
        mReturnDistance = lerpValue(0.05f, mReturnDistance, mStoredDistance);
    }

    mAt = (1.0f - rate) * mReturnLookAtPos + rate * mStoredLookAtPos;
    mEye = mAt + mReturnDistance * mReturnDir;
    updateReturnStep();
}

/**
 * Follows the target actor, or moves back to the stored camera while returning.
 */
void CameraPoserFixActor::exeFollow() {
    if (!mIsReturn && mIsReturnEnd) {
        if (!mIsReturnToDir) {
            mEye.set(mStoredCameraPos);
            mAt.set(mStoredLookAtPos);
            return;
        }

        if (!mIsReturnGoalItemAppear) {
            mEye = mReturnGoalLookAtPos + mStoredDistance * mStoredDir;
            mAt.set(mReturnGoalLookAtPos);
            return;
        }

        mEye = mStoredLookAtPos + mReturnDistance * mReturnDir;
        mAt = mStoredLookAtPos;
        return;
    }

    if (_170 || isCalcEndAfterInterpole()) {
        return;
    }

    if (!mIsReturn) {
        if (!mIsDirectAngle) {
            calcFixActorCameraPose(this, mDistance, mTargetActor, mOffset, mAngleH, mAngleV,
                                   mIsCalcNearestAtFromPreAt, mStoredLookAtPos);
            return;
        }

        calcFixActorCameraPoseDirect(this, mDistance, mTargetActor, mOffset, mAngleV,
                                     &mDirectAngleDir);
        return;
    }

    if (mReturnStepMax == 0) {
        mIsReturn = false;
        mIsReturnEnd = true;

        if (!mIsReturnToDir) {
            mEye.set(mStoredCameraPos.x, mStoredCameraPos.y + mReturnOffsetY, mStoredCameraPos.z);
            mAt.set(mStoredLookAtPos);
            return;
        }

        mEye = mReturnGoalLookAtPos + mStoredDistance * mStoredDir;
        mAt.set(mReturnGoalLookAtPos);
        return;
    }

    if (mIsReturnToDir) {
        if (mIsReturnGoalItemAppear) {
            updateReturnLerpDistance(calcReturnRate(mReturnStep, mReturnStepMax));
            return;
        }

        mReturnDistance = lerpValue(mReturnDistanceRate, mReturnDistance, mStoredDistance);
        lerpVec(&mReturnDir, mReturnDir, mStoredDir, mReturnDirRate);
        mReturnDir.normalize();
        lerpVec(&mReturnLookAtPos, mReturnLookAtPos, mReturnGoalLookAtPos, 0.1f);
        mEye = mReturnLookAtPos + mReturnDistance * mReturnDir;
        mAt.set(mReturnLookAtPos);
        updateReturnStep();
        return;
    }

    if (mIsReturnLerpDir) {
        lerpVec(&mReturnDir, mReturnDir, mStoredDir, 0.1f);
        mReturnDir.normalize();
        updateReturnLerpDistance(calcReturnRate(mReturnStep, mReturnStepMax));
        return;
    }

    sead::Vector3f goalCameraPos = {mStoredCameraPos.x, mStoredCameraPos.y + mReturnOffsetY,
                                    mStoredCameraPos.z};
    f32 rate = calcReturnRate(mReturnStep, mReturnStepMax);

    mEye = goalCameraPos * rate + (1.0f - rate) * mReturnStartCameraPos;
    mAt = (1.0f - rate) * mReturnLookAtPos + rate * mStoredLookAtPos;

    if (mReturnStep >= mReturnStepMax) {
        mIsReturn = false;
        mIsReturnEnd = true;
        mEye = goalCameraPos;
        mAt.set(mStoredLookAtPos);
        return;
    }

    mReturnStep++;
}

/**
 * Unused state.
 */
void CameraPoserFixActor::exeGoIn() {}

/**
 * Unused state.
 */
void CameraPoserFixActor::exeGoOut() {}

/**
 * Updates the look at position the camera returns to.
 * @param rTarget New look at position.
 */
void CameraPoserFixActor::updateReturnCamTarget(sead::Vector3f& rTarget) {
    if (mIsReturnLerpDir || mIsReturnToDir) {
        mReturnLookAtPos = rTarget;
    } else {
        mStoredLookAtPos = rTarget;
    }
}

/**
 * Starts returning from the current camera to the stored camera.
 * @param pCamera Camera user to read the current camera from.
 * @param step Number of frames the return takes.
 * @param isKeepDir Whether to keep the direction of the current camera.
 */
void CameraPoserFixActor::setReturn(const IUseCamera_RS* pCamera, s32 step, bool isKeepDir) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnToDir = false;
    mIsReturnLerpDir = true;
    mIsReturnGoalItemAppear = false;
    mReturnStartCameraPos.set(getCameraPos_RS(pCamera, 0));
    mReturnLookAtPos.set(getCameraAt_RS(pCamera, 0));
    mReturnDistance = (mReturnLookAtPos - mReturnStartCameraPos).length();

    if (step > 0 && isKeepDir) {
        mReturnDir = mReturnStartCameraPos - mReturnLookAtPos;
        mReturnDir.normalize();
        mStoredDir = mReturnDir;
    } else {
        mReturnDir = mStoredCameraPos - mStoredLookAtPos;
        mReturnDir.normalize();
    }
}

/**
 * Starts returning from the current camera after a goal item appeared.
 * @param pCamera Camera user to read the current camera from.
 * @param step Number of frames the return takes.
 * @param isKeepDir Whether to keep the direction of the current camera.
 */
void CameraPoserFixActor::setReturnGoalItemAppear(const IUseCamera_RS* pCamera, s32 step,
                                                  bool isKeepDir) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnLerpDir = false;
    mIsReturnGoalItemAppear = true;
    mReturnStartCameraPos.set(getCameraPos_RS(pCamera, 0));
    mReturnLookAtPos.set(getCameraAt_RS(pCamera, 0));

    if (step > 0 && isKeepDir) {
        mReturnDir = mReturnStartCameraPos - mReturnLookAtPos;
        mIsReturnToDir = true;
        mReturnDir.normalize();
        mReturnDistance = (mReturnLookAtPos - mReturnStartCameraPos).length();
    } else {
        mIsReturnToDir = false;
    }
}

/**
 * Starts returning from the given camera to the stored camera.
 * @param pCamera Unused.
 * @param step Number of frames the return takes.
 * @param rCameraPos Camera position to start from.
 * @param rLookAtPos Look at position to start from.
 */
void CameraPoserFixActor::setReturn(const IUseCamera_RS* pCamera, s32 step,
                                    const sead::Vector3f& rCameraPos,
                                    const sead::Vector3f& rLookAtPos) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnToDir = false;
    mIsReturnGoalItemAppear = false;
    mReturnStartCameraPos.set(rCameraPos);
    mReturnLookAtPos.set(rLookAtPos);
}

/**
 * Starts returning to the given camera.
 * @param step Number of frames the return takes.
 * @param rCameraPos Camera position to return to.
 * @param rLookAtPos Look at position to return to.
 */
void CameraPoserFixActor::setReturn(s32 step, const sead::Vector3f& rCameraPos,
                                    const sead::Vector3f& rLookAtPos) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnToDir = false;
    mIsReturnGoalItemAppear = false;
    mReturnStartCameraPos.set(rCameraPos);
    mReturnLookAtPos.set(rCameraPos);
    mStoredCameraPos.set(rCameraPos);
    mStoredLookAtPos.set(rLookAtPos);
}

/**
 * Starts returning to the stored look at position along a direction.
 * @param step Number of frames the return takes.
 * @param rCameraPos Camera position to start from.
 * @param dir Direction to start from.
 */
void CameraPoserFixActor::setReturnKeepDir(s32 step, const sead::Vector3f& rCameraPos,
                                           sead::Vector3f dir) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mReturnDir = dir;
    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnToDir = true;
    mIsReturnGoalItemAppear = false;
    mReturnDir.normalize();
    mReturnStartCameraPos.set(rCameraPos);
    mReturnLookAtPos.set(mStoredLookAtPos);
    mReturnDistance = (mReturnLookAtPos - mReturnStartCameraPos).length();
    mStoredDir = mReturnStartCameraPos - mReturnLookAtPos;
    mStoredDir.normalize();
}

/**
 * Starts returning to a look at position along a direction given by angles.
 * @param step Number of frames the return takes.
 * @param rCameraPos Camera position to start from.
 * @param rLookAtPos Look at position to return to.
 * @param rStartLookAtPos Look at position to start from.
 * @param dir Direction to start from.
 * @param angleH Horizontal angle of the goal direction in degrees.
 * @param angleV Vertical angle of the goal direction in degrees.
 * @param dirRate Interpolation rate of the direction.
 * @param distanceRate Interpolation rate of the distance.
 * @param distance Goal distance.
 */
void CameraPoserFixActor::setReturnSetDir(s32 step, const sead::Vector3f& rCameraPos,
                                          const sead::Vector3f& rLookAtPos,
                                          const sead::Vector3f& rStartLookAtPos,
                                          sead::Vector3f dir, f32 angleH, f32 angleV,
                                          f32 dirRate, f32 distanceRate, f32 distance) {
    if (mIsReturn && !mIsReturnEnd) {
        return;
    }

    mReturnDir = dir;
    mIsReturn = true;
    mIsReturnEnd = false;
    mReturnStepMax = step;
    mReturnStep = 0;
    mIsReturnToDir = true;
    mIsReturnGoalItemAppear = false;
    mReturnDir.normalize();
    mReturnStartCameraPos.set(rCameraPos);
    mReturnGoalLookAtPos.set(rLookAtPos);
    mReturnLookAtPos.set(rStartLookAtPos);
    mReturnDistance = (mReturnLookAtPos - mReturnStartCameraPos).length();
    mReturnDistanceRate = distanceRate;
    mStoredDistance = distance;
    mReturnDirRate = dirRate;
    mStoredDir.x = sinf(sead::Mathf::deg2rad(angleH));
    mStoredDir.y = sinf(sead::Mathf::deg2rad(angleV));
    mStoredDir.z = cosf(sead::Mathf::deg2rad(angleH));
}

/**
 * Creates a camera for talking to an actor.
 * @param pActor Actor to talk to.
 */
CameraPoserFixTalk::CameraPoserFixTalk(const LiveActor* pActor) : CameraPoserFixActor(pActor) {}

/**
 * Turns to the side of the actor the previous camera was on and places the camera.
 * @param rInfo Start info.
 */
void CameraPoserFixTalk::start(const CameraStartInfo& rInfo) {
    f32 angleH = mTalkAngleH;
    const LiveActor* actor = mTargetActor;

    sead::Vector3f preCameraDir = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcPreCameraDirH(&preCameraDir, this);
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, actor);

    mAngleH =
        calcAngleOnPlaneDegree(front, preCameraDir, sead::Vector3f::ey) >= 0.0f ? angleH : -angleH;
    calcFixActorCameraPose(this, mDistance, mTargetActor, mOffset, mAngleH, mAngleV,
                           mIsCalcNearestAtFromPreAt, mStoredLookAtPos);
}

/**
 * Creates a camera for fishing.
 * @param pActor Actor to look at.
 */
CameraPoserFixFishing::CameraPoserFixFishing(const LiveActor* pActor)
    : CameraPoserFixActor(pActor) {}

/**
 * Sets the angle and the offsets used on either side of the actor.
 * @param angleH Horizontal angle in degrees.
 * @param rOffset Offset used when the camera is on the left side.
 * @param rOffsetRev Offset used when the camera is on the right side.
 */
void CameraPoserFixFishing::initParam(f32 angleH, const sead::Vector3f& rOffset,
                                      const sead::Vector3f& rOffsetRev) {
    mFishingAngleH = angleH;
    mFishingOffset.set(rOffset);
    mFishingOffsetRev.set(rOffsetRev);
}

/**
 * Turns to the side of the actor the previous camera was on and places the camera.
 * @param rInfo Start info.
 */
void CameraPoserFixFishing::start(const CameraStartInfo& rInfo) {
    const LiveActor* actor = mTargetActor;

    sead::Vector3f preCameraDir = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcPreCameraDirH(&preCameraDir, this);
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, actor);

    f32 angle = calcAngleOnPlaneDegree(front, preCameraDir, sead::Vector3f::ey);
    f32 angleH = mFishingAngleH;

    if (angle >= 0.0f) {
        mOffset = mFishingOffset;
    } else {
        mOffset = mFishingOffsetRev;
        angleH = -angleH;
    }

    mAngleH = angleH;

    calcFixActorCameraPose(this, mDistance, mTargetActor, mOffset, mAngleH, mAngleV,
                           mIsCalcNearestAtFromPreAt, mStoredLookAtPos);
}

}  // namespace al
