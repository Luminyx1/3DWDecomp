#include "Library/MapObj/RailMoveMapParts.hpp"

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Movement/RailMoveMovement.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Project/AreaObj/SwitchOnAreaGroup.hpp"
#include "Project/Collision/CollisionParts.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(RailMoveMapParts, StandBy)
NERVE_ACTION_IMPL(RailMoveMapParts, MoveSign)
NERVE_ACTION_IMPL(RailMoveMapParts, Move)

NERVE_ACTIONS_MAKE_STRUCT(RailMoveMapParts, StandBy, MoveSign, Move)
}  // namespace

namespace al {
/**
 * Constructs a rail moving map part.
 * @param pName actor name
 */
RailMoveMapParts::RailMoveMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its rail movement.
 * @param rInfo actor init info
 */
void RailMoveMapParts::init(const ActorInitInfo& rInfo) {
    using RailMoveMapPartsFunctor = FunctorV0M<RailMoveMapParts*, void (RailMoveMapParts::*)()>;

    initNerveAction(this, "StandBy", &NrvRailMoveMapParts.collector, 1);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    registerAreaHostMtx(this, rInfo);
    if (isExistRail(this)) {
        f32 radius = getClippingRadius(this);
        setSyncRailToNearestPos(this);
        setRailClippingInfo(&mRailPos, this, 100.0f, radius);
    }

    mRailMoveMovement = new RailMoveMovement(this, rInfo);
    initNerveState(this, mRailMoveMovement, NrvRailMoveMapParts.Move.data(), "レール移動");
    if (!listenStageSwitchOnStart(this, RailMoveMapPartsFunctor(this, &RailMoveMapParts::start))) {
        start();
    }

    if (listenStageSwitchOnStop(this, RailMoveMapPartsFunctor(this, &RailMoveMapParts::stop))) {
        stop();
    }

    if (isExistShadow(this)) {
        bool isShadow = false;
        tryGetArg(&isShadow, rInfo, "IsShadow");
        if (!isShadow) {
            invalidateShadow(this);
        }
    }

    mSwitchKeepOnAreaGroup = tryCreateSwitchKeepOnAreaGroup(this, rInfo);
    mSwitchOnAreaGroup = tryCreateSwitchOnAreaGroup(this, rInfo);
    trySyncStageSwitchAppear(this);
    _142 = true;
    tryGetArg(&mIsAlwaysUpdateCollMtx, rInfo, "AlwaysUpdateCollMtx");
}

/**
 * Starts moving when the start switch turns on.
 */
void RailMoveMapParts::start() {
    if (isNerve(this, NrvRailMoveMapParts.StandBy.data())) {
        startNerveAction(this, "MoveSign");
    }
}

/**
 * Stops moving when the stop switch turns on.
 */
void RailMoveMapParts::stop() {
    if (isNerve(this, NrvRailMoveMapParts.Move.data())) {
        startNerveAction(this, "StandBy");
    }
}

/**
 * Shows or hides the model on request.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool RailMoveMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
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
 * Updates the switch areas and, if requested, the collision matrix.
 */
void RailMoveMapParts::control() {
    if (mSwitchKeepOnAreaGroup) {
        mSwitchKeepOnAreaGroup->update(getTrans(this));
    }

    if (mSwitchOnAreaGroup) {
        mSwitchOnAreaGroup->update(getTrans(this));
    }

    if (mIsAlwaysUpdateCollMtx && (!mCollisionParts->_160 || !mCollisionParts->_161)) {
        sead::Matrix34f mtx;
        makeMtxSRT(&mtx, this);
        mCollisionParts->forceResetAllMtxAndSetUpdateMtxOneTime(mtx);
    }
}

/**
 * Waits for the start switch.
 */
void RailMoveMapParts::exeStandBy() {}

/**
 * Plays the move sign action.
 */
void RailMoveMapParts::exeMoveSign() {
    if ((isFirstStep(this) && !tryStartAction(this, "MoveSign")) || isActionEnd(this)) {
        startNerveAction(this, "Move");
    }
}

/**
 * Moves along the rail.
 */
void RailMoveMapParts::exeMove() {
    updateNerveState(this);
}
}  // namespace al
