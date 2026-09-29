#include "Library/MapObj/SwingMapParts.hpp"

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
#include "Project/AreaObj/AreaObjUtil.hpp"

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
     * @brief Constructs a map part that swings back and forth around one of its local axes.
     * @param pName The actor name.
     */
    SwingMapParts::SwingMapParts(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the swing movement, the map part and its start conditions.
     * @param rInfo The actor init info.
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
     * @brief Starts swinging when the start switch turns on.
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
     * @brief Starts swinging on floor touch and handles show/hide model messages.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the message was handled.
     */
    bool SwingMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
        if (mIsFloorTouchStart && isMsgFloorTouch(pMsg) && isNerve(this, NrvSwingMapParts.StandBy.data())) {
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
     * @brief Applies the current swing angle.
     */
    void SwingMapParts::control() {
        rotateQuatLocalDirDegree(this, mStartQuat, mRotateAxis, mSwingMovement->getCurrentAngle());
    }

    /**
     * @brief Waits for the start condition.
     */
    void SwingMapParts::exeStandBy() {}

    /**
     * @brief Swings to the right until the swing stops.
     */
    void SwingMapParts::exeMoveRight() {
        mSwingMovement->updateNerve();
        if (mSwingMovement->isStop()) {
            startNerveAction(this, "Stop");
        }
    }

    /**
     * @brief Swings to the left until the swing stops.
     */
    void SwingMapParts::exeMoveLeft() {
        mSwingMovement->updateNerve();
        if (mSwingMovement->isStop()) {
            startNerveAction(this, "Stop");
        }
    }

    /**
     * @brief Waits at the end of a swing, then swings in the other direction.
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
