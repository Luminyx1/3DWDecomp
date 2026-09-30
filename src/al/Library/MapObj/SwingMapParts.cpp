#include "Library/MapObj/SwingMapParts.hpp"

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ChildStep.hpp"
#include "Library/Movement/SwingMovement.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(SwingMapParts, StandBy)
NERVE_ACTION_IMPL(SwingMapParts, MoveRight)
NERVE_ACTION_IMPL(SwingMapParts, MoveLeft)
NERVE_ACTION_IMPL(SwingMapParts, Stop)

NERVE_ACTIONS_MAKE_STRUCT(SwingMapParts, StandBy, MoveRight, MoveLeft, Stop)
}  // namespace

namespace al {
/**
 * Constructs a swinging map part.
 * @param pName actor name
 */
SwingMapParts::SwingMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its swing movement.
 * @param rInfo actor init info
 */
void SwingMapParts::init(const ActorInitInfo& rInfo) {
    mSwingMovement = new SwingMovement(rInfo);
    if (mSwingMovement->isLeft()) {
        initNerveAction(this, "MoveLeft", &NrvSwingMapParts.collector, 0);
    } else {
        initNerveAction(this, "MoveRight", &NrvSwingMapParts.collector, 0);
    }

    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    mStartQuat = getQuat(this);
    createChildStep(rInfo, this, true);
    tryGetArg(&mRotateAxis, rInfo, "RotateAxis");
    tryGetArg(&mIsFloorTouchStart, rInfo, "IsFloorTouchStart");
    if (mIsFloorTouchStart ||
        listenStageSwitchOnStart(this, FunctorV0M<SwingMapParts*, void (SwingMapParts::*)()>(
                                           this, &SwingMapParts::start))) {
        startNerveAction(this, "StandBy");
    }

    trySyncStageSwitchAppear(this);
}

/**
 * Starts swinging when the start switch turns on.
 */
void SwingMapParts::start() {
    if (!isNerve(this, NrvSwingMapParts.StandBy.data())) {
        return;
    }

    if (mSwingMovement->isLeft()) {
        initNerveAction(this, "MoveLeft", &NrvSwingMapParts.collector, 0);
    } else {
        initNerveAction(this, "MoveRight", &NrvSwingMapParts.collector, 0);
    }
}

/**
 * Starts swinging when touched, and handles model visibility messages.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool SwingMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (mIsFloorTouchStart && isMsgFloorTouch(pMsg) &&
        isNerve(this, NrvSwingMapParts.StandBy.data())) {
        if (mSwingMovement->isLeft()) {
            initNerveAction(this, "MoveLeft", &NrvSwingMapParts.collector, 0);
        } else {
            initNerveAction(this, "MoveRight", &NrvSwingMapParts.collector, 0);
        }

        return true;
    }

    if (isMsgShowModel(pMsg)) {
        showModelIfHide(this);
        return true;
    }

    if (isMsgHideModel(pMsg)) {
        hideModelIfShow(this);
        return true;
    }

    return false;
}

/**
 * Applies the swing angle.
 */
void SwingMapParts::control() {
    rotateQuatLocalDirDegree(this, mStartQuat, mRotateAxis, mSwingMovement->getCurrentAngle());
}

/**
 * Waits for the start.
 */
void SwingMapParts::exeStandBy() {}

/**
 * Swings to the right.
 */
void SwingMapParts::exeMoveRight() {
    mSwingMovement->updateNerve();
    if (mSwingMovement->isStop()) {
        startNerveAction(this, "Stop");
    }
}

/**
 * Swings to the left.
 */
void SwingMapParts::exeMoveLeft() {
    mSwingMovement->updateNerve();
    if (mSwingMovement->isStop()) {
        startNerveAction(this, "Stop");
    }
}

/**
 * Stops at the end of a swing.
 */
void SwingMapParts::exeStop() {
    mSwingMovement->updateNerve();
    if (mSwingMovement->isStop()) {
        return;
    }

    if (mSwingMovement->isLeft()) {
        startNerveAction(this, "MoveLeft");
    } else {
        startNerveAction(this, "MoveRight");
    }
}
}  // namespace al
