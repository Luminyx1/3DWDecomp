#include "Library/MapObj/RotateMapParts.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ChildStep.hpp"
#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Audio/SeFunction.hpp"

namespace {
    using namespace al;

    NERVE_ACTION_IMPL(RotateMapParts, StandBy)
    NERVE_ACTION_IMPL(RotateMapParts, Rotate)
    NERVE_ACTION_IMPL_(RotateMapParts, EffectOnAngle, StandBy)
    NERVE_ACTION_IMPL(RotateMapParts, AssistStop)
    NERVE_ACTION_IMPL(RotateMapParts, AssistStopSync)

    NERVE_ACTIONS_MAKE_STRUCT(RotateMapParts, StandBy, Rotate, EffectOnAngle, AssistStop, AssistStopSync)
}  // namespace

namespace al {
    /**
     * @brief Constructs a map part that keeps rotating around one of its local axes.
     * @param pName The actor name.
     */
    RotateMapParts::RotateMapParts(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the map part, its rotation parameters and optional angle triggered effect.
     * @param rInfo The actor init info.
     */
    void RotateMapParts::init(const ActorInitInfo& rInfo) {
        initNerveAction(this, "Rotate", &NrvRotateMapParts.collector, 0);
        initActorPoseTQSV(this);
        initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
        registerAreaHostMtx(this, rInfo);

        if (mHitSensorKeeper != nullptr) {
            mIsSupportFreezeSync = registSupportFreezeSyncGroup(this, rInfo);
        }

        tryGetArg(&mRotateAxis, rInfo, "RotateAxis");
        tryGetArg(&mRotateSpeed, rInfo, "RotateSpeed");
        createChildStep(rInfo, this, true);

        if (listenStageSwitchOnStart(this, FunctorV0M<RotateMapParts*, void (RotateMapParts::*)()>(
                                               this, &RotateMapParts::start))) {
            startNerveAction(this, "StandBy");
        }

        trySyncStageSwitchAppear(this);
        mIsResetOnAppear = rInfo._78;
        mStartTrans = getTrans(this);
        mStartQuat = getQuat(this);

        tryGetArg(&mIsTriggerEffectOnAngle, rInfo, "IsTriggerEffectOnAngle");
        if (mIsTriggerEffectOnAngle) {
            tryGetArg(&mEffectTriggerAngle, rInfo, "EffectTriggerAngle");
            mAngle = 0.0f;
        }

        _142 = true;
        _143 = true;
    }

    /**
     * @brief Starts rotating when the start switch turns on.
     */
    void RotateMapParts::start() {
        if (isNerve(this, NrvRotateMapParts.StandBy.data())) {
            startNerveAction(this, "Rotate");
        }
    }

    /**
     * @brief Appears, optionally restoring the initial pose.
     */
    void RotateMapParts::appear() {
        if (mIsResetOnAppear) {
            setQuat(this, mStartQuat);
            setTrans(this, mStartTrans);
            mAngle = 0.0f;
            if (isNerve(this, NrvRotateMapParts.StandBy.data())) {
                startNerveAction(this, "Rotate");
            }
        }
        LiveActor::appear();
    }

    /**
     * @brief Dies, optionally going back to the stand by nerve.
     */
    void RotateMapParts::kill() {
        if (mIsResetOnAppear) {
            startNerveAction(this, "StandBy");
        }
        LiveActor::kill();
    }

    /**
     * @brief Handles assist touches, support freeze sync and show/hide model messages.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the message was handled.
     */
    bool RotateMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
        if (isMsgTouchAssist(pMsg)) {
            mAssistTimer = 45;
            return true;
        }

        if (mIsSupportFreezeSync) {
            if (isMsgIsNerveSupportFreeze(pMsg)) {
                return isNerve(this, NrvRotateMapParts.AssistStop.data());
            }

            if (isMsgOnSyncSupportFreeze(pMsg)) {
                if (!isNerve(this, NrvRotateMapParts.AssistStop.data())) {
                    if (isExistAction(this)) {
                        stopAction(this);
                    }
                    startNerveAction(this, "AssistStopSync");
                }
                return true;
            }

            if (isMsgOffSyncSupportFreeze(pMsg)) {
                if (isNerve(this, NrvRotateMapParts.AssistStopSync.data())) {
                    if (isExistAction(this)) {
                        restartAction(this);
                    }
                    startNerveAction(this, "Rotate");
                }
                return true;
            }
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
     * @brief Waits for the start switch.
     */
    void RotateMapParts::exeStandBy() {}

    /**
     * @brief Rotates, plays the rotation sound and triggers the angle effect.
     */
    void RotateMapParts::exeRotate() {
        rotateQuatLocalDirDegree(this, mRotateAxis, mRotateSpeed / 100.0f);

        if (mAssistTimer > 0) {
            startNerveAction(this, "AssistStop");
        }

        if (isExistSePlayNameInUserInfo(this, "RotateWithSpeed")) {
            tryHoldSeWithParam(this, "RotateWithSpeed", mRotateSpeed, nullptr);
        }

        if (mIsTriggerEffectOnAngle) {
            f32 speed = mRotateSpeed / 100.0f;
            f32 angle = mAngle + speed;
            if (angle >= 360.0f) {
                angle += -360.0f;
            }
            mAngle = angle;
            if (!_140 && mAngle < mEffectTriggerAngle && mAngle + speed >= mEffectTriggerAngle) {
                tryStartEffectAction(this, "EffectOnAngle");
            }
        }
    }

    /**
     * @brief Stops while the assist timer runs, then resumes rotating.
     */
    void RotateMapParts::exeAssistStop() {
        mAssistTimer--;
        if (mAssistTimer <= 0) {
            mAssistTimer = 0;
            startNerveAction(this, "Rotate");
        }

        if (mIsTriggerEffectOnAngle && mAngle < mEffectTriggerAngle &&
            mAngle + mRotateSpeed / 100.0f >= mEffectTriggerAngle) {
            tryStartEffectAction(this, "EffectOnAngle");
        }
    }

    /**
     * @brief Stays stopped while another member of the support freeze sync group is stopped.
     */
    void RotateMapParts::exeAssistStopSync() {}
}  // namespace al
