#include "Library/MapObj/RollingCubeMapParts.hpp"

#include <math/seadBoundBox.h>

#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/EffectMtxSetter.hpp"
#include "Library/Math/Axis.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Obj/PartsModel.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Joint/RollingCubePoseKeeper.hpp"
#include "Project/Joint/RollingCubePoseKeeperUtil.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(RollingCubeMapParts, Wait)
NERVE_ACTION_IMPL(RollingCubeMapParts, Start)
NERVE_ACTION_IMPL(RollingCubeMapParts, Rotate)
NERVE_ACTION_IMPL(RollingCubeMapParts, Fall)
NERVE_ACTION_IMPL_(RollingCubeMapParts, SlideX, Slide)
NERVE_ACTION_IMPL_(RollingCubeMapParts, SlideY, Slide)
NERVE_ACTION_IMPL_(RollingCubeMapParts, SlideZ, Slide)
NERVE_ACTION_IMPL_(RollingCubeMapParts, LandX, Land)
NERVE_ACTION_IMPL_(RollingCubeMapParts, LandY, Land)
NERVE_ACTION_IMPL_(RollingCubeMapParts, LandZ, Land)
NERVE_ACTION_IMPL_(RollingCubeMapParts, FallLandX, FallLand)
NERVE_ACTION_IMPL_(RollingCubeMapParts, FallLandY, FallLand)
NERVE_ACTION_IMPL_(RollingCubeMapParts, FallLandZ, FallLand)
NERVE_ACTION_IMPL(RollingCubeMapParts, Stop)

NERVE_ACTIONS_MAKE_STRUCT(RollingCubeMapParts, Wait, Start, Rotate, Fall, SlideX, SlideY, SlideZ,
                          LandX, LandY, LandZ, FallLandX, FallLandY, FallLandZ, Stop)
}  // namespace

namespace al {
/**
 * Constructs a rolling cube map part.
 * @param pName actor name
 */
RollingCubeMapParts::RollingCubeMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the cube, its move limit model and its pose keeper.
 * @param rInfo actor init info
 */
void RollingCubeMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvRollingCubeMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    mInitialPoseQuat = getQuat(this);
    mInitialPoseTrans = getTrans(this);
    bool isUseMoveLimit = false;
    tryGetArg(&isUseMoveLimit, rInfo, "IsUseMoveLimit");
    if (isUseMoveLimit) {
        mMoveLimitMtx = new sead::Matrix34f();
        mMoveLimitMtx->makeQT(mInitialPoseQuat, mInitialPoseTrans);
        mMoveLimitPartsModel = new PartsModel("");
        StringTmp<256> model;
        StringTmp<256> archive;
        makeMapPartsModelName(&model, &archive, rInfo);
        mMoveLimitPartsModel->initPartsSuffix(this, rInfo, model.cstr(), "MoveLimit",
                                              mMoveLimitMtx, false);
    }

    sead::BoundBox3f boundBox;
    if (isExistModelResourceYaml(this, "BoxInfo", nullptr) &&
        tryGetByamlBox3f(&boundBox, ByamlIter(getModelResourceYaml(this, "BoxInfo", nullptr)))) {
        mRollingCubePoseKeeper = createRollingCubePoseKeeper(boundBox, rInfo);
    } else {
        mRollingCubePoseKeeper = createRollingCubePoseKeeper(this, rInfo);
    }

    bool isFloorTouchStart = true;
    tryGetArg(&isFloorTouchStart, rInfo, "IsFloorTouchStart");
    if (!isFloorTouchStart) {
        startNerveAction(this, "Start");
    }

    mEffectMtxSetter = tryCreateEffectMtxSetter(this, "EffectMtxSetter");
    if (mEffectMtxSetter) {
        mEffectMtxSetter->setMtxPtr(&mLandEffectMtx, "LandEffectMtx");
    }

    f32 clippingRadius = 0.0f;
    calcRollingCubeClippingInfo(&mClippingTrans, &clippingRadius, mRollingCubePoseKeeper, 0.0f);
    setClippingInfo(this, clippingRadius, &mClippingTrans);
    tryListenStageSwitchKill(this);
    makeActorAppeared();
}

/**
 * Plays the disappear reaction and kills the cube.
 */
void RollingCubeMapParts::kill() {
    startHitReactionDisappear(this);
    LiveActor::kill();
}

/**
 * Starts rolling when the player touches the cube.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool RollingCubeMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgPlayerFloorTouch(pMsg) && isNerve(this, NrvRollingCubeMapParts.Wait.data())) {
        startNerveAction(this, "Start");
        return true;
    }

    return false;
}

/**
 * Updates the move limit and land effect matrices.
 */
void RollingCubeMapParts::control() {
    if (mMoveLimitMtx) {
        mMoveLimitMtx->makeQT(mInitialPoseQuat, getTrans(this));
    }

    calcMtxLandEffect(&mLandEffectMtx, mRollingCubePoseKeeper, getQuat(this), getTrans(this));
}

/**
 * Resets the cube to its start pose and appears.
 */
void RollingCubeMapParts::appearAndSetStart() {
    mRollingCubePoseKeeper->setStart();
    setQuat(this, mInitialPoseQuat);
    setTrans(this, mInitialPoseTrans);
    resetPosition(this, false);
    setNerveNextMovement(isNextFallKey());
    appear();
}

/**
 * Starts the next movement of the current key.
 * @param isNextFallKey whether the next key is a fall
 */
void RollingCubeMapParts::setNerveNextMovement(bool isNextFallKey) {
    if (isMovementCurrentKeyRotate(mRollingCubePoseKeeper)) {
        startNerveAction(this, "Rotate");
        return;
    }

    if (isNextFallKey) {
        startNerveAction(this, "Fall");
        return;
    }

    s32 axis = sead::Mathi::abs(
        static_cast<s32>(calcNearVecFromAxis3(nullptr, sead::Vector3f::ey, getQuat(this))));
    switch (axis) {
    case static_cast<s32>(Axis::X):
        startNerveAction(this, "SlideX");
        return;
    case static_cast<s32>(Axis::Z):
        startNerveAction(this, "SlideZ");
        return;
    case static_cast<s32>(Axis::Y):
        startNerveAction(this, "SlideY");
        return;
    default:
        return;
    }
}

/**
 * Checks if the next key is a fall.
 * @return whether the next key is a fall
 */
bool RollingCubeMapParts::isNextFallKey() const {
    bool isNextFall = false;
    tryGetArg(&isNextFall, getCurrentKeyPlacementInfo(mRollingCubePoseKeeper), "isNextFall");
    return isNextFall;
}

/**
 * Waits for the player.
 */
void RollingCubeMapParts::exeWait() {}

/**
 * Waits before the first movement.
 */
void RollingCubeMapParts::exeStart() {
    if (isGreaterEqualStep(this, 40)) {
        setNerveNextMovement(isNextFallKey());
    }
}

/**
 * Rotates to the next key.
 */
void RollingCubeMapParts::exeRotate() {
    if (isFirstStep(this)) {
        fittingToCurrentKeyBoundingBox(getQuatPtr(this), getTransPtr(this), mRollingCubePoseKeeper);
        mCurrentPoseQuat = getQuat(this);
        mCurrentPoseTrans = getTrans(this);
        mMovementTime = getMovementTime() - 1;
    }

    calcCurrentKeyQT(getQuatPtr(this), getTransPtr(this), mRollingCubePoseKeeper, mCurrentPoseQuat,
                     mCurrentPoseTrans, calcNerveSquareInRate(this, mMovementTime));
    if (isGreaterEqualStep(this, mMovementTime)) {
        if (nextRollingCubeKey(mRollingCubePoseKeeper)) {
            if (!isNextFallKey()) {
                setNerveNextLand();
            } else if (isMovementCurrentKeyRotate(mRollingCubePoseKeeper)) {
                startNerveAction(this, "Rotate");
            } else {
                startNerveAction(this, "Fall");
            }
        } else {
            startNerveAction(this, "Stop");
        }
    }
}

/**
 * Gets the movement time of the current key.
 * @return movement time
 */
s32 RollingCubeMapParts::getMovementTime() const {
    s32 movementTime = 40;
    tryGetArg(&movementTime, getCurrentKeyPlacementInfo(mRollingCubePoseKeeper), "MovementTime");
    return movementTime;
}

/**
 * Starts the land nerve of the current axis.
 */
void RollingCubeMapParts::setNerveNextLand() {
    s32 axis = sead::Mathi::abs(
        static_cast<s32>(calcNearVecFromAxis3(nullptr, sead::Vector3f::ey, getQuat(this))));
    switch (axis) {
    case static_cast<s32>(Axis::X):
        startNerveAction(this, "LandX");
        return;
    case static_cast<s32>(Axis::Z):
        startNerveAction(this, "LandZ");
        return;
    case static_cast<s32>(Axis::Y):
        startNerveAction(this, "LandY");
        return;
    default:
        return;
    }
}

/**
 * Slides to the next key.
 */
void RollingCubeMapParts::exeSlide() {
    if (updateSlide()) {
        if (nextRollingCubeKey(mRollingCubePoseKeeper)) {
            if (!isNextFallKey()) {
                setNerveNextLand();
            } else if (isMovementCurrentKeyRotate(mRollingCubePoseKeeper)) {
                startNerveAction(this, "Rotate");
            } else {
                startNerveAction(this, "Fall");
            }
        } else {
            startNerveAction(this, "Stop");
        }
    }
}

/**
 * Updates the slide towards the next key.
 * @return whether the slide ended
 */
bool RollingCubeMapParts::updateSlide() {
    if (isFirstStep(this)) {
        fittingToCurrentKeyBoundingBox(getQuatPtr(this), getTransPtr(this), mRollingCubePoseKeeper);
        mCurrentPoseQuat = getQuat(this);
        mCurrentPoseTrans = getTrans(this);
        mMovementTime = getMovementTime() - 1;
    }

    calcCurrentKeyQT(getQuatPtr(this), getTransPtr(this), mRollingCubePoseKeeper, mCurrentPoseQuat,
                     mCurrentPoseTrans, calcNerveSquareInRate(this, mMovementTime));
    return isGreaterEqualStep(this, mMovementTime);
}

/**
 * Falls to the next key.
 */
void RollingCubeMapParts::exeFall() {
    if (updateSlide()) {
        if (nextRollingCubeKey(mRollingCubePoseKeeper)) {
            if (!isNextFallKey()) {
                setNerveNextFallLand();
            } else if (isMovementCurrentKeyRotate(mRollingCubePoseKeeper)) {
                startNerveAction(this, "Rotate");
            } else {
                startNerveAction(this, "Fall");
            }
        } else {
            startNerveAction(this, "Stop");
        }
    }
}

/**
 * Starts the fall land nerve of the current axis.
 */
void RollingCubeMapParts::setNerveNextFallLand() {
    s32 axis = sead::Mathi::abs(
        static_cast<s32>(calcNearVecFromAxis3(nullptr, sead::Vector3f::ey, getQuat(this))));
    switch (axis) {
    case static_cast<s32>(Axis::X):
        startNerveAction(this, "FallLandX");
        return;
    case static_cast<s32>(Axis::Z):
        startNerveAction(this, "FallLandZ");
        return;
    case static_cast<s32>(Axis::Y):
        startNerveAction(this, "FallLandY");
        return;
    default:
        return;
    }
}

/**
 * Lands after a slide or rotation.
 */
void RollingCubeMapParts::exeLand() {
    if (isGreaterEqualStep(this, getLandTime() - 1)) {
        setNerveNextMovement(false);
    }
}

/**
 * Gets the land time of the current key.
 * @return land time
 */
s32 RollingCubeMapParts::getLandTime() const {
    s32 landTime = 40;
    tryGetArg(&landTime, getCurrentKeyPlacementInfo(mRollingCubePoseKeeper), "LandTime");
    return landTime;
}

/**
 * Lands after a fall.
 */
void RollingCubeMapParts::exeFallLand() {
    if (isGreaterEqualStep(this, getLandTime())) {
        setNerveNextMovement(false);
    }
}

/**
 * Stops at the last key, disappearing if stoppable.
 */
void RollingCubeMapParts::exeStop() {
    if (mIsStoppable) {
        if (isFirstStep(this)) {
            startHitReactionDisappear(this);
        }

        if (isStep(this, 1)) {
            setQuat(this, mInitialPoseQuat);
            setTrans(this, mInitialPoseTrans);
            resetPosition(this, false);
            makeActorDead();
        }
    }
}

/**
 * Checks if the cube stopped.
 * @return whether the cube stopped
 */
bool RollingCubeMapParts::isStop() const {
    return isNerve(this, NrvRollingCubeMapParts.Stop.data());
}
}  // namespace al
