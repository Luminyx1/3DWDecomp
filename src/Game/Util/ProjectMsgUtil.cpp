#include "Util/ProjectMsgUtil.hpp"

#include <math/seadVector.h>
#include <prim/seadRuntimeTypeInfo.h>

#include "Library/HitSensor/SensorMsg.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Util/ControlUserUtil.hpp"

namespace al {
class BlockRailRider;
class ComboCounter;

// Messages owned by ActionLibrary that carry a combo counter. They are only defined inside
// ActorSensorUtil.cpp, so the ones tryGetMsgComboCount() casts to are repeated here.
SENSOR_MSG_WITH_DATA(Explosion, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(ExplosionCollide, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerKouraAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(KickKouraAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(KickKouraAttackCollide, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackTrample, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDropReflect, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDropHighJump, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackStatueDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjStatueDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjStatueDropReflect, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackInvincibleAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerInvincibleTouch, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackTailAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BallAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BallTrample, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBodyAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBodyLanding, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackSlidingAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerCooperationHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerGiantHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackGiant, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackClimbAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackClimbSliding, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBoomerangAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackSpinAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BlockUpperPunch, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BlockLowerPunch, ComboCounter*, ComboCounter);
}  // namespace al

// Like SENSOR_MSG_WITH_DATA, but the vector is taken and returned by reference.
#define SENSOR_MSG_WITH_VECTOR(Type, DataName)                                                     \
    class SensorMsg##Type : public al::SensorMsg {                                                 \
        SEAD_RTTI_OVERRIDE(SensorMsg##Type, al::SensorMsg)                                         \
    public:                                                                                        \
        SensorMsg##Type(const sead::Vector3f& rData) : m##DataName(rData) {}                       \
        ~SensorMsg##Type() override = default;                                                     \
        const sead::Vector3f& get##DataName() const { return m##DataName; }                      \
                                                                                                   \
    private:                                                                                       \
        sead::Vector3f m##DataName;                                                                \
    }

namespace rc {
SENSOR_MSG(AskControlUserId1);
SENSOR_MSG(AskControlUserId2);
SENSOR_MSG(AskControlUserId3);
SENSOR_MSG(AskControlUserId4);
SENSOR_MSG_WITH_DATA(BlockRailRide, al::BlockRailRider*, BlockRailRider);
SENSOR_MSG_WITH_VECTOR(CameraPush, PushVec);
SENSOR_MSG_WITH_DATA(DashPanel, s32, Time);
SENSOR_MSG_WITH_DATA(ModifiedDashPanel, s32, Time);
SENSOR_MSG_WITH_DATA(FlingPoleDash, s32, Time);
SENSOR_MSG(AskBobsledDashPanel);
SENSOR_MSG(BobsledBodyAttack);
SENSOR_MSG_WITH_DATA(BobsledTrample, al::ComboCounter*, ComboCounter);
SENSOR_MSG(BossGorobonAttack);
SENSOR_MSG(BossGorobonSpinShot);
SENSOR_MSG(BoxKillerBulletNoTouch);
SENSOR_MSG(EnemyFloorTouchTrampoline);
SENSOR_MSG(GongShockwave);
SENSOR_MSG_WITH_DATA(GorobonAttack, al::ComboCounter*, ComboCounter);
SENSOR_MSG(ImozoTouch);
SENSOR_MSG(DonketsuSlidePush);
SENSOR_MSG_WITH_DATA(JumpPanelAction, bool, SuperJump);
SENSOR_MSG(KillerMagnumExplosion);
SENSOR_MSG(KillerShockWave);
SENSOR_MSG(KillerTouch);
SENSOR_MSG(PackunEat);
SENSOR_MSG(PackunEatStart);
SENSOR_MSG(PackunPush);
SENSOR_MSG(PackunThrowAttack);
SENSOR_MSG_WITH_VECTOR(ItemReflect, HitDir);
SENSOR_MSG(PanelNoteHipDrop);
SENSOR_MSG(PlayerCheckpointTouch);
SENSOR_MSG_WITH_VECTOR(RouteDokanPlayerTouch, Front);
SENSOR_MSG(RouteDokanPlayerReflectNoDamage);
SENSOR_MSG(RouteDokanPlayerReflect);
SENSOR_MSG(RouteDokanItemGet);
SENSOR_MSG(RouteDokanPlayerAttack);
SENSOR_MSG(SkateShoesAttack);
SENSOR_MSG(SpinnerAttack);
SENSOR_MSG(TentackMagmaBallBreak);
SENSOR_MSG(TuccondorAttack);
SENSOR_MSG_WITH_VECTOR(ByugoWind, Power);
SENSOR_MSG_WITH_VECTOR(GustWind, Power);
SENSOR_MSG(BullAttack);
SENSOR_MSG(RaidonAttack);
SENSOR_MSG(TakoboBulletAttack);
SENSOR_MSG(DossunPress);
SENSOR_MSG(GoalKillRunaway);
SENSOR_MSG(GroundSnapOffForce);
SENSOR_MSG_WITH_VECTOR(AddForce, Force);
SENSOR_MSG(RequestTouchFromHoldedPlayer);
SENSOR_MSG(RingBeamerSign);
SENSOR_MSG(ItemBubbleBreak);
SENSOR_MSG(ItemBubbleBreakAndGetItem);
SENSOR_MSG(RouteDokanKouraAttack);
SENSOR_MSG(BubbleAttack);
SENSOR_MSG(NokonokoKick);
SENSOR_MSG_WITH_DATA(RequestPlayerGetReaction, const char*, Name);
SENSOR_MSG(BombBoundKickedAttack);
SENSOR_MSG(BubbleVanish);
SENSOR_MSG(IsEnableExitStage);
SENSOR_MSG(IsEnableIslandWarp);
SENSOR_MSG(IsDisableCancelBubble);
SENSOR_MSG_WITH_DATA(QueryHostPlayer, const al::LiveActor*, HostPlayer);
SENSOR_MSG(NeedleRollerAttack);
SENSOR_MSG(BoundTrampoline);
SENSOR_MSG(MeraWanwanPush);
SENSOR_MSG(MeraWanwanAttack);
SENSOR_MSG(KoopaLastBreakObj);
SENSOR_MSG(KoopaLastReactionObj);
SENSOR_MSG(StartGoalDemoPole);
SENSOR_MSG(GhostPresentGet);
SENSOR_MSG(InkTouch);
SENSOR_MSG(PlayerGigaStep);
SENSOR_MSG(FireRollerAttack);
SENSOR_MSG_WITH_VECTOR(PushDir, Dir);
SENSOR_MSG(PushConnected);
SENSOR_MSG_WITH_DATA(Graffiti, s32, Type);
SENSOR_MSG(NpcBindInit);
SENSOR_MSG(NpcBindCancel);
SENSOR_MSG(RaidonBreakLightReaction);
SENSOR_MSG(TouchAssistBurnPeto);
class SensorMsgNeedleRollerHit : public al::SensorMsg {
    SEAD_RTTI_OVERRIDE(SensorMsgNeedleRollerHit, al::SensorMsg)

public:
    SensorMsgNeedleRollerHit(const sead::Vector3f& rDir, f32 power)
        : mDir(rDir), mPower(power) {}

    ~SensorMsgNeedleRollerHit() override = default;

    const sead::Vector3f& getDir() const { return mDir; }

    f32 getPower() const { return mPower; }

private:
    sead::Vector3f mDir;
    f32 mPower;
};

SENSOR_MSG(StartGoalDemoHouse);
SENSOR_MSG(StartDemoBossStart);
SENSOR_MSG_WITH_VECTOR(DebugMove, Pos);

/**
 * Copies a vector out of a message as a plain copy of its storage.
 * @param pDst The destination vector.
 * @param rSrc The source vector.
 */
static inline void copyVector(sead::Vector3f* pDst, const sead::Vector3f& rSrc) {
    pDst->e = rSrc.e;
}

/**
 * Sends a message from one sensor to the host of another.
 * @param rMsg The message.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
static inline bool sendMsgSensorToSensor(const al::SensorMsg& rMsg, al::HitSensor* pReceiver,
                                         al::HitSensor* pSender) {
    return al::getSensorHost(pReceiver)->receiveMsg(&rMsg, pSender, pReceiver);
}

/**
 * Sends the AskControlUserId message matching a control user to a sensor's own host.
 * @param pSensor The sensor that both sends and receives the message.
 * @param userId The control user ID to ask about (0-3).
 * @return Whether the message was received, false for an out-of-range ID.
 */
bool sendMsgAskControlUserId(al::HitSensor* pSensor, s32 userId) {
    switch (userId) {
    case 0:
        return sendMsgSensorToSensor(SensorMsgAskControlUserId1(), pSensor, pSensor);
    case 1:
        return sendMsgSensorToSensor(SensorMsgAskControlUserId2(), pSensor, pSensor);
    case 2:
        return sendMsgSensorToSensor(SensorMsgAskControlUserId3(), pSensor, pSensor);
    case 3:
        return sendMsgSensorToSensor(SensorMsgAskControlUserId4(), pSensor, pSensor);
    default:
        return false;
    }
}

/**
 * Sends a BlockRailRide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pBlockRailRider The rider that rides the block rail.
 * @return Whether the message was received.
 */
bool sendMsgBlockRailRide(al::HitSensor* pReceiver, al::HitSensor* pSender,
                          al::BlockRailRider* pBlockRailRider) {
    return sendMsgSensorToSensor(SensorMsgBlockRailRide(pBlockRailRider), pReceiver, pSender);
}

/**
 * Sends a CameraPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rPushVec The push vector.
 * @return Whether the message was received.
 */
bool sendMsgCameraPush(al::HitSensor* pReceiver, al::HitSensor* pSender,
                       const sead::Vector3f& rPushVec) {
    return sendMsgSensorToSensor(SensorMsgCameraPush(rPushVec), pReceiver, pSender);
}

/**
 * Sends a DashPanel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param time The dash time.
 * @return Whether the message was received.
 */
bool sendMsgDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time) {
    return sendMsgSensorToSensor(SensorMsgDashPanel(time), pReceiver, pSender);
}

/**
 * Sends a ModifiedDashPanel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param time The dash time.
 * @return Whether the message was received.
 */
bool sendMsgModifiedDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time) {
    return sendMsgSensorToSensor(SensorMsgModifiedDashPanel(time), pReceiver, pSender);
}

/**
 * Sends a FlingPoleDash message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param time The dash time.
 * @return Whether the message was received.
 */
bool sendMsgFlingPoleDash(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time) {
    return sendMsgSensorToSensor(SensorMsgFlingPoleDash(time), pReceiver, pSender);
}

/**
 * Sends the right block message to an object above a block that was punched from below.
 * @param pReceiver The receiving sensor.
 * @param pSender The block's sensor.
 * @param userId The control user that hit the block, negative if none.
 * @param pComboCounter The combo counter forwarded to enemies and NPCs.
 * @return Whether a message was received.
 */
bool trySendMsgBlockToUpperObj(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 userId,
                               al::ComboCounter* pComboCounter) {
    if (!al::isSensorEye(pSender) || userId < 0) {
        return false;
    }

    if (al::isSensorEnemy(pReceiver) || al::isSensorNpc(pReceiver)) {
        return al::sendMsgBlockUpperPunch(pReceiver, pSender, pComboCounter);
    }

    if (al::isSensorMapObj(pReceiver)) {
        bool isItemGet = al::sendMsgBlockItemGet(pReceiver, pSender);
        return isItemGet | al::sendMsgBlockUpperPunch(pReceiver, pSender, nullptr);
    }

    if (al::isSensorPlayer(pReceiver) && findRelativeControlUserId(pReceiver) != userId) {
        return al::sendMsgBlockUpperPunch(pReceiver, pSender, nullptr);
    }

    return false;
}

/**
 * Sends a BlockLowerPunch message to an enemy below a block.
 * @param pReceiver The receiving sensor.
 * @param pSender The block's sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool trySendMsgBlockToLowerObj(al::HitSensor* pReceiver, al::HitSensor* pSender,
                               al::ComboCounter* pComboCounter) {
    if (al::isSensorEye(pSender) && al::isSensorEnemy(pReceiver)) {
        return al::sendMsgBlockLowerPunch(pReceiver, pSender, pComboCounter);
    }

    return false;
}

/**
 * Sends a AskBobsledDashPanel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgAskBobsledDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgAskBobsledDashPanel(), pReceiver, pSender);
}

/**
 * Sends a BobsledBodyAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBobsledBodyAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBobsledBodyAttack(), pReceiver, pSender);
}

/**
 * Sends a BobsledTrample message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter.
 * @return Whether the message was received.
 */
bool sendMsgBobsledTrample(al::HitSensor* pReceiver, al::HitSensor* pSender,
                           al::ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBobsledTrample(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BossGorobonAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBossGorobonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBossGorobonAttack(), pReceiver, pSender);
}

/**
 * Sends a BossGorobonSpinShot message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBossGorobonSpinShot(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBossGorobonSpinShot(), pReceiver, pSender);
}

/**
 * Sends a BoxKillerBulletNoTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBoxKillerBulletNoTouch(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBoxKillerBulletNoTouch(), pReceiver, pSender);
}

/**
 * Sends a EnemyFloorTouchTrampoline message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyFloorTouchTrampoline(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyFloorTouchTrampoline(), pReceiver, pSender);
}

/**
 * Sends a GongShockwave message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGongShockwave(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGongShockwave(), pReceiver, pSender);
}

/**
 * Sends a GorobonAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter.
 * @return Whether the message was received.
 */
bool sendMsgGorobonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender,
                          al::ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgGorobonAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a ImozoTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgImozoTouch(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgImozoTouch(), pReceiver, pSender);
}

/**
 * Sends a DonketsuSlidePush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgDonketsuSlidePush(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgDonketsuSlidePush(), pReceiver, pSender);
}

/**
 * Sends a JumpPanelAction message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param isSuperJump Whether the panel triggers a super jump.
 * @return Whether the message was received.
 */
bool sendMsgJumpPanelAction(al::HitSensor* pReceiver, al::HitSensor* pSender, bool isSuperJump) {
    return sendMsgSensorToSensor(SensorMsgJumpPanelAction(isSuperJump), pReceiver, pSender);
}

/**
 * Sends a KillerMagnumExplosion message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerMagnumExplosion(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerMagnumExplosion(), pReceiver, pSender);
}

/**
 * Sends a KillerShockWave message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerShockWave(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerShockWave(), pReceiver, pSender);
}

/**
 * Sends a KillerTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerTouch(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerTouch(), pReceiver, pSender);
}

/**
 * Sends a PackunEat message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPackunEat(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPackunEat(), pReceiver, pSender);
}

/**
 * Sends a PackunEatStart message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPackunEatStart(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPackunEatStart(), pReceiver, pSender);
}

/**
 * Sends a PackunPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPackunPush(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPackunPush(), pReceiver, pSender);
}

/**
 * Sends a PackunThrowAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPackunThrowAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPackunThrowAttack(), pReceiver, pSender);
}

/**
 * Sends a ItemReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rHitDir The reflection hit direction.
 * @return Whether the message was received.
 */
bool sendMsgItemReflect(al::HitSensor* pReceiver, al::HitSensor* pSender,
                        const sead::Vector3f& rHitDir) {
    return sendMsgSensorToSensor(SensorMsgItemReflect(rHitDir), pReceiver, pSender);
}

/**
 * Sends a PanelNoteHipDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPanelNoteHipDrop(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPanelNoteHipDrop(), pReceiver, pSender);
}

/**
 * Sends a PlayerCheckpointTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCheckpointTouch(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCheckpointTouch(), pReceiver, pSender);
}

/**
 * Sends a RouteDokanPlayerTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rFront The front direction of the pipe.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanPlayerTouch(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                  const sead::Vector3f& rFront) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanPlayerTouch(rFront), pReceiver, pSender);
}

/**
 * Sends a RouteDokanPlayerReflectNoDamage message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanPlayerReflectNoDamage(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanPlayerReflectNoDamage(), pReceiver, pSender);
}

/**
 * Sends a RouteDokanPlayerReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanPlayerReflect(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanPlayerReflect(), pReceiver, pSender);
}

/**
 * Sends a RouteDokanItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanItemGet(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanItemGet(), pReceiver, pSender);
}

/**
 * Sends a RouteDokanPlayerAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanPlayerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanPlayerAttack(), pReceiver, pSender);
}

/**
 * Sends a SkateShoesAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSkateShoesAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSkateShoesAttack(), pReceiver, pSender);
}

/**
 * Sends a SpinnerAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSpinnerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSpinnerAttack(), pReceiver, pSender);
}

/**
 * Sends a TentackMagmaBallBreak message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgTentackMagmaBallBreak(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgTentackMagmaBallBreak(), pReceiver, pSender);
}

/**
 * Sends a TuccondorAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgTuccondorAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgTuccondorAttack(), pReceiver, pSender);
}

/**
 * Sends a ByugoWind message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rPower The wind power.
 * @return Whether the message was received.
 */
bool sendMsgByugoWind(al::HitSensor* pReceiver, al::HitSensor* pSender,
                      const sead::Vector3f& rPower) {
    return sendMsgSensorToSensor(SensorMsgByugoWind(rPower), pReceiver, pSender);
}

/**
 * Sends a GustWind message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rPower The wind power.
 * @return Whether the message was received.
 */
bool sendMsgGustWind(al::HitSensor* pReceiver, al::HitSensor* pSender,
                     const sead::Vector3f& rPower) {
    return sendMsgSensorToSensor(SensorMsgGustWind(rPower), pReceiver, pSender);
}

/**
 * Sends a BullAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBullAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBullAttack(), pReceiver, pSender);
}

/**
 * Sends a RaidonAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRaidonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRaidonAttack(), pReceiver, pSender);
}

/**
 * Sends a TakoboBulletAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgTakoboBulletAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgTakoboBulletAttack(), pReceiver, pSender);
}

/**
 * Sends a DossunPress message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgDossunPress(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgDossunPress(), pReceiver, pSender);
}

/**
 * Sends a GoalKillRunaway message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGoalKillRunaway(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGoalKillRunaway(), pReceiver, pSender);
}

/**
 * Sends a GroundSnapOffForce message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGroundSnapOffForce(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGroundSnapOffForce(), pReceiver, pSender);
}

/**
 * Sends a AddForce message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rForce The force to add.
 * @return Whether the message was received.
 */
bool sendMsgAddForce(al::HitSensor* pReceiver, al::HitSensor* pSender,
                     const sead::Vector3f& rForce) {
    return sendMsgSensorToSensor(SensorMsgAddForce(rForce), pReceiver, pSender);
}

/**
 * Sends a RequestTouchFromHoldedPlayer message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRequestTouchFromHoldedPlayer(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRequestTouchFromHoldedPlayer(), pReceiver, pSender);
}

/**
 * Sends a RingBeamerSign message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRingBeamerSign(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRingBeamerSign(), pReceiver, pSender);
}

/**
 * Sends a ItemBubbleBreak message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgItemBubbleBreak(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgItemBubbleBreak(), pReceiver, pSender);
}

/**
 * Sends a ItemBubbleBreakAndGetItem message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgItemBubbleBreakAndGetItem(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgItemBubbleBreakAndGetItem(), pReceiver, pSender);
}

/**
 * Sends a RouteDokanKouraAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRouteDokanKouraAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRouteDokanKouraAttack(), pReceiver, pSender);
}

/**
 * Sends a BubbleAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBubbleAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBubbleAttack(), pReceiver, pSender);
}

/**
 * Sends a NokonokoKick message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNokonokoKick(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNokonokoKick(), pReceiver, pSender);
}

/**
 * Sends a RequestPlayerGetReaction message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pName The reaction name.
 * @return Whether the message was received.
 */
bool sendMsgRequestPlayerGetReaction(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                     const char* pName) {
    return sendMsgSensorToSensor(SensorMsgRequestPlayerGetReaction(pName), pReceiver, pSender);
}

/**
 * Sends a BombBoundKickedAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBombBoundKickedAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBombBoundKickedAttack(), pReceiver, pSender);
}

/**
 * Sends a BubbleVanish message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBubbleVanish(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBubbleVanish(), pReceiver, pSender);
}

/**
 * Sends a IsEnableExitStage message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsEnableExitStage(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsEnableExitStage(), pReceiver, pSender);
}

/**
 * Sends a IsEnableIslandWarp message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsEnableIslandWarp(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsEnableIslandWarp(), pReceiver, pSender);
}

/**
 * Sends a IsDisableCancelBubble message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsDisableCancelBubble(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsDisableCancelBubble(), pReceiver, pSender);
}

/**
 * Sends a QueryHostPlayer message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pHostPlayer The host player.
 * @return Whether the message was received.
 */
bool sendMsgQueryHostPlayer(al::HitSensor* pReceiver, al::HitSensor* pSender,
                            const al::LiveActor* pHostPlayer) {
    return sendMsgSensorToSensor(SensorMsgQueryHostPlayer(pHostPlayer), pReceiver, pSender);
}

/**
 * Sends a NeedleRollerAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNeedleRollerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNeedleRollerAttack(), pReceiver, pSender);
}

/**
 * Sends a NeedleRollerHit message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rDir The hit direction.
 * @param power The hit power.
 * @return Whether the message was received.
 */
bool sendMsgNeedleRollerHit(al::HitSensor* pReceiver, al::HitSensor* pSender,
                            const sead::Vector3f& rDir, f32 power) {
    return sendMsgSensorToSensor(SensorMsgNeedleRollerHit(rDir, power), pReceiver, pSender);
}

/**
 * Sends a BoundTrampoline message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBoundTrampoline(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBoundTrampoline(), pReceiver, pSender);
}

/**
 * Sends a MeraWanwanPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgMeraWanwanPush(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgMeraWanwanPush(), pReceiver, pSender);
}

/**
 * Sends a MeraWanwanAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgMeraWanwanAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgMeraWanwanAttack(), pReceiver, pSender);
}

/**
 * Sends a KoopaLastBreakObj message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKoopaLastBreakObj(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKoopaLastBreakObj(), pReceiver, pSender);
}

/**
 * Sends a KoopaLastReactionObj message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKoopaLastReactionObj(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKoopaLastReactionObj(), pReceiver, pSender);
}

/**
 * Sends a StartGoalDemoPole message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgStartGoalDemoPole(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgStartGoalDemoPole(), pReceiver, pSender);
}

/**
 * Sends a GhostPresentGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGhostPresentGet(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGhostPresentGet(), pReceiver, pSender);
}

/**
 * Sends a InkTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgInkTouch(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgInkTouch(), pReceiver, pSender);
}

/**
 * Sends a PlayerGigaStep message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerGigaStep(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerGigaStep(), pReceiver, pSender);
}

/**
 * Sends a FireRollerAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgFireRollerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgFireRollerAttack(), pReceiver, pSender);
}

/**
 * Sends a PushDir message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rDir The push direction.
 * @return Whether the message was received.
 */
bool sendMsgPushDir(al::HitSensor* pReceiver, al::HitSensor* pSender, const sead::Vector3f& rDir) {
    return sendMsgSensorToSensor(SensorMsgPushDir(rDir), pReceiver, pSender);
}

/**
 * Sends a PushConnected message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPushConnected(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPushConnected(), pReceiver, pSender);
}

/**
 * Sends a Graffiti message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param type The graffiti type.
 * @return Whether the message was received.
 */
bool sendMsgGraffiti(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 type) {
    return sendMsgSensorToSensor(SensorMsgGraffiti(type), pReceiver, pSender);
}

/**
 * Sends a NpcBindInit message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNpcBindInit(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNpcBindInit(), pReceiver, pSender);
}

/**
 * Sends a NpcBindCancel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNpcBindCancel(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNpcBindCancel(), pReceiver, pSender);
}

/**
 * Sends a RaidonBreakLightReaction message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRaidonBreakLightReaction(al::HitSensor* pReceiver, al::HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRaidonBreakLightReaction(), pReceiver, pSender);
}

/**
 * Sends a NeedleRollerHit message to a sensor that may be null.
 * @param pReceiver The receiving sensor, may be null.
 * @param pSender The sending sensor.
 * @param rDir The hit direction.
 * @param power The hit power.
 * @return Whether the message was received.
 */
static inline bool trySendMsgNeedleRollerHit(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                             const sead::Vector3f& rDir, f32 power) {
    if (pReceiver == nullptr) {
        return false;
    }

    return sendMsgNeedleRollerHit(pReceiver, pSender, rDir, power);
}

/**
 * Sends a NeedleRollerHit message to every sensor the actor collided with.
 * @param pActor The actor whose ground, wall and ceiling sensors receive the message.
 * @param pSender The sending sensor.
 * @param rDir The hit direction.
 * @param power The hit power.
 * @return Whether any of the messages was received.
 */
bool sendMsgNeedleRollerHitToCollition(al::LiveActor* pActor, al::HitSensor* pSender,
                                       const sead::Vector3f& rDir, f32 power) {
    bool isReceived = trySendMsgNeedleRollerHit(al::tryGetCollidedGroundSensor(pActor), pSender,
                                                rDir, power);

    if (trySendMsgNeedleRollerHit(al::tryGetCollidedWallSensor(pActor), pSender, rDir, power)) {
        isReceived = true;
    }

    if (trySendMsgNeedleRollerHit(al::tryGetCollidedCeilingSensor(pActor), pSender, rDir,
                                  power)) {
        isReceived = true;
    }

    return isReceived;
}

/**
 * Gets the block rail rider carried by a BlockRailRide message.
 * @param pMsg The message.
 * @return The rider, or nullptr if the message is not BlockRailRide.
 */
al::BlockRailRider* tryGetMsgParamBlockRailRider(const al::SensorMsg* pMsg) {
    const SensorMsgBlockRailRide* pRideMsg = sead::DynamicCast<const SensorMsgBlockRailRide>(pMsg);

    if (pRideMsg != nullptr) {
        return pRideMsg->getBlockRailRider();
    }

    return nullptr;
}

/**
 * Checks whether a message is the AskControlUserId message of a control user.
 * @param pMsg The message.
 * @param userId The control user ID (0-3).
 * @return Whether the message has that type.
 */
bool isMsgAskControlUserId(const al::SensorMsg* pMsg, s32 userId) {
    if (userId < 0) {
        return false;
    }

    switch (userId) {
    case 0:
        return sead::IsDerivedFrom<SensorMsgAskControlUserId1>(pMsg);
    case 1:
        return sead::IsDerivedFrom<SensorMsgAskControlUserId2>(pMsg);
    case 2:
        return sead::IsDerivedFrom<SensorMsgAskControlUserId3>(pMsg);
    case 3:
        return sead::IsDerivedFrom<SensorMsgAskControlUserId4>(pMsg);
    default:
        return false;
    }
}

/**
 * Checks whether a message is the AskControlUserId message of the control user owning a sensor.
 * @param pMsg The message.
 * @param pSensor The sensor whose control user is checked.
 * @return Whether the message has that type, false for a null sensor.
 */
bool isMsgAskControlUserId(const al::SensorMsg* pMsg, const al::HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return false;
    }

    return isMsgAskControlUserId(pMsg, tryFindControlUserId(pSensor));
}

/**
 * Checks whether a message is BlockRailRide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBlockRailRide(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBlockRailRide>(pMsg);
}

/**
 * Checks whether a message is CameraPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgCameraPush(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgCameraPush>(pMsg);
}

/**
 * Checks whether a message is DashPanel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDashPanel(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDashPanel>(pMsg);
}

/**
 * Checks whether a message is ModifiedDashPanel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgModifiedDashPanel(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgModifiedDashPanel>(pMsg);
}

/**
 * Checks whether a message is FlingPoleDash.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgFlingPoleDash(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgFlingPoleDash>(pMsg);
}

/**
 * Checks whether a message makes a coin be collected.
 * @param pMsg The message.
 * @return Whether the message is an item get or a block item get.
 */
bool isMsgCoinGet(const al::SensorMsg* pMsg) {
    return al::isMsgItemGetAll(pMsg) || al::isMsgBlockItemGet(pMsg);
}

/**
 * Checks whether a message is AskBobsledDashPanel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgAskBobsledDashPanel(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgAskBobsledDashPanel>(pMsg);
}

/**
 * Checks whether a message is BobsledBodyAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBobsledBodyAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBobsledBodyAttack>(pMsg);
}

/**
 * Checks whether a message is BobsledTrample.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBobsledTrample(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBobsledTrample>(pMsg);
}

/**
 * Checks whether a message is BossGorobonAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBossGorobonAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBossGorobonAttack>(pMsg);
}

/**
 * Checks whether a message is BossGorobonSpinShot.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBossGorobonSpinShot(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBossGorobonSpinShot>(pMsg);
}

/**
 * Checks whether a message is BoxKillerBulletNoTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBoxKillerBulletNoTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBoxKillerBulletNoTouch>(pMsg);
}

/**
 * Checks whether a message is EnemyFloorTouchTrampoline.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyFloorTouchTrampoline(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyFloorTouchTrampoline>(pMsg);
}

/**
 * Checks whether a message is GongShockwave.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGongShockwave(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGongShockwave>(pMsg);
}

/**
 * Checks whether a message is GorobonAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGorobonAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGorobonAttack>(pMsg);
}

/**
 * Checks whether a message is ImozoTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgImozoTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgImozoTouch>(pMsg);
}

/**
 * Checks whether a message is DonketsuSlidePush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDonketsuSlidePush(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDonketsuSlidePush>(pMsg);
}

/**
 * Checks whether a message is ItemReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemReflect(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgItemReflect>(pMsg);
}

/**
 * Checks whether a message is JumpPanelAction.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgJumpPanelAction(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgJumpPanelAction>(pMsg);
}

/**
 * Checks whether a message is KillerMagnumExplosion.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerMagnumExplosion(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerMagnumExplosion>(pMsg);
}

/**
 * Checks whether a message is KillerShockWave.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerShockWave(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerShockWave>(pMsg);
}

/**
 * Checks whether a message is KillerTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerTouch>(pMsg);
}

/**
 * Checks whether a message is PackunEat.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPackunEat(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPackunEat>(pMsg);
}

/**
 * Checks whether a message is PackunEatStart.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPackunEatStart(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPackunEatStart>(pMsg);
}

/**
 * Checks whether a message is PackunPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPackunPush(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPackunPush>(pMsg);
}

/**
 * Checks whether a message is PackunThrowAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPackunThrowAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPackunThrowAttack>(pMsg);
}

/**
 * Checks whether a message is PanelNoteHipDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPanelNoteHipDrop(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPanelNoteHipDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerCheckpointTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCheckpointTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCheckpointTouch>(pMsg);
}

/**
 * Checks whether a message is RouteDokanPlayerAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanPlayerAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanPlayerAttack>(pMsg);
}

/**
 * Checks whether a message is RouteDokanPlayerTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanPlayerTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanPlayerTouch>(pMsg);
}

/**
 * Checks whether a message is RouteDokanPlayerReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanPlayerReflect(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanPlayerReflect>(pMsg);
}

/**
 * Checks whether a message is RouteDokanPlayerReflectNoDamage.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanPlayerReflectNoDamage(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanPlayerReflectNoDamage>(pMsg);
}

/**
 * Checks whether a message is RouteDokanItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanItemGet(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanItemGet>(pMsg);
}

/**
 * Checks whether a message is SpinnerAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSpinnerAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSpinnerAttack>(pMsg);
}

/**
 * Checks whether a message is SkateShoesAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSkateShoesAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSkateShoesAttack>(pMsg);
}

/**
 * Checks whether a message is TentackMagmaBallBreak.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTentackMagmaBallBreak(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTentackMagmaBallBreak>(pMsg);
}

/**
 * Checks whether a message is TuccondorAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTuccondorAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTuccondorAttack>(pMsg);
}

/**
 * Checks whether a message is ByugoWind.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgByugoWind(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgByugoWind>(pMsg);
}

/**
 * Checks whether a message is BullAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBullAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBullAttack>(pMsg);
}

/**
 * Checks whether a message is RaidonAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRaidonAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRaidonAttack>(pMsg);
}

/**
 * Checks whether a message is TakoboBulletAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTakoboBulletAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTakoboBulletAttack>(pMsg);
}

/**
 * Checks whether a message is DossunPress.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDossunPress(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDossunPress>(pMsg);
}

/**
 * Checks whether a message is GoalKillRunaway.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGoalKillRunaway(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGoalKillRunaway>(pMsg);
}

/**
 * Checks whether a message is GroundSnapOffForce.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGroundSnapOffForce(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGroundSnapOffForce>(pMsg);
}

/**
 * Checks whether a message is AddForce.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgAddForce(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgAddForce>(pMsg);
}

/**
 * Checks whether a message is RequestTouchFromHoldedPlayer.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRequestTouchFromHoldedPlayer(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRequestTouchFromHoldedPlayer>(pMsg);
}

/**
 * Checks whether a message is RingBeamerSign.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRingBeamerSign(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRingBeamerSign>(pMsg);
}

/**
 * Checks whether a message is ItemBubbleBreak.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemBubbleBreak(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgItemBubbleBreak>(pMsg);
}

/**
 * Checks whether a message is ItemBubbleBreakAndGetItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemBubbleBreakAndGetItem(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgItemBubbleBreakAndGetItem>(pMsg);
}

/**
 * Checks whether a message is RouteDokanKouraAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRouteDokanKouraAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRouteDokanKouraAttack>(pMsg);
}

/**
 * Checks whether a message is BubbleAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBubbleAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBubbleAttack>(pMsg);
}

/**
 * Checks whether a message is NokonokoKick.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNokonokoKick(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNokonokoKick>(pMsg);
}

/**
 * Checks whether a message is RequestPlayerGetReaction.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRequestPlayerGetReaction(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRequestPlayerGetReaction>(pMsg);
}

/**
 * Checks whether a message is BombBoundKickedAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBombBoundKickedAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBombBoundKickedAttack>(pMsg);
}

/**
 * Checks whether a message is BubbleVanish.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBubbleVanish(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBubbleVanish>(pMsg);
}

/**
 * Checks whether a message is TouchAssistBurnPeto.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistBurnPeto(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssistBurnPeto>(pMsg);
}

/**
 * Checks whether a message is IsEnableExitStage.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsEnableExitStage(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsEnableExitStage>(pMsg);
}

/**
 * Checks whether a message is IsEnableIslandWarp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsEnableIslandWarp(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsEnableIslandWarp>(pMsg);
}

/**
 * Checks whether a message is IsDisableCancelBubble.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsDisableCancelBubble(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsDisableCancelBubble>(pMsg);
}

/**
 * Checks whether a message is QueryHostPlayer.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgQueryHostPlayer(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgQueryHostPlayer>(pMsg);
}

/**
 * Checks whether a message is NeedleRollerAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNeedleRollerAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNeedleRollerAttack>(pMsg);
}

/**
 * Checks whether a message is NeedleRollerHit.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNeedleRollerHit(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNeedleRollerHit>(pMsg);
}

/**
 * Checks whether a message is BoundTrampoline.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBoundTrampoline(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBoundTrampoline>(pMsg);
}

/**
 * Checks whether a message is MeraWanwanPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgMeraWanwanPush(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgMeraWanwanPush>(pMsg);
}

/**
 * Checks whether a message is MeraWanwanAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgMeraWanwanAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgMeraWanwanAttack>(pMsg);
}

/**
 * Checks whether a message is KoopaLastBreakObj.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKoopaLastBreakObj(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKoopaLastBreakObj>(pMsg);
}

/**
 * Checks whether a message is KoopaLastReactionObj.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKoopaLastReactionObj(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKoopaLastReactionObj>(pMsg);
}

/**
 * Checks whether a message is StartGoalDemoPole.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgStartGoalDemoPole(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgStartGoalDemoPole>(pMsg);
}

/**
 * Checks whether a message is StartGoalDemoHouse.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgStartGoalDemoHouse(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgStartGoalDemoHouse>(pMsg);
}

/**
 * Checks whether a message is StartDemoBossStart.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgStartDemoBossStart(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgStartDemoBossStart>(pMsg);
}

/**
 * Checks whether a message is GhostPresentGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGhostPresentGet(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGhostPresentGet>(pMsg);
}

/**
 * Checks whether a BindInit message has the normal bind type.
 * @param pMsg The BindInit message.
 * @return Whether the bind type is normal.
 */
bool isMsgBindInitNormal(const al::SensorMsg* pMsg) {
    return al::getBindInitType(pMsg) == 0;
}

/**
 * Checks whether a BindInit message has the request bind type.
 * @param pMsg The BindInit message.
 * @return Whether the bind type is request.
 */
bool isMsgBindInitRequest(const al::SensorMsg* pMsg) {
    return al::getBindInitType(pMsg) == 1;
}

/**
 * Checks whether a BindInit message has the giant bind type.
 * @param pMsg The BindInit message.
 * @return Whether the bind type is giant.
 */
bool isMsgBindInitGiant(const al::SensorMsg* pMsg) {
    return al::getBindInitType(pMsg) == 2;
}

/**
 * Checks whether a message is InkTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgInkTouch(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgInkTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerGigaStep.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerGigaStep(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerGigaStep>(pMsg);
}

/**
 * Checks whether a message is PushDir.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPushDir(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPushDir>(pMsg);
}

/**
 * Checks whether a message is PushConnected.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPushConnected(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPushConnected>(pMsg);
}

/**
 * Checks whether a message is Graffiti.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGraffiti(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGraffiti>(pMsg);
}

/**
 * Checks whether a message is FireRollerAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgFireRollerAttack(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgFireRollerAttack>(pMsg);
}

/**
 * Checks whether a message is NpcBindInit.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNpcBindInit(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNpcBindInit>(pMsg);
}

/**
 * Checks whether a message is NpcBindCancel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNpcBindCancel(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNpcBindCancel>(pMsg);
}

/**
 * Checks whether a message is RaidonBreakLightReaction.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRaidonBreakLightReaction(const al::SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRaidonBreakLightReaction>(pMsg);
}

/**
 * Checks whether a sensor belongs to a Captain Toad brigade NPC.
 * @param pSensor The sensor, may be null.
 * @return Whether the sensor's host has the Brigade sub name.
 */
bool isSensorKinopioBrigadeNpc(const al::HitSensor* pSensor) {
    if (pSensor != nullptr && al::getSensorHost(pSensor) != nullptr &&
        al::isSensorHostSubName(pSensor, "Brigade")) {
        return true;
    }

    return false;
}

/**
 * Gets the combo counter carried by a message.
 * @param pMsg The message.
 * @return The combo counter, or nullptr if the message has none.
 */
al::ComboCounter* getMsgComboCount(const al::SensorMsg* pMsg) {
    return tryGetMsgComboCount(pMsg);
}

/**
 * Gets the combo counter carried by any of the attack messages that have one.
 * @param pMsg The message.
 * @return The combo counter, or nullptr if the message has none.
 */
al::ComboCounter* tryGetMsgComboCount(const al::SensorMsg* pMsg) {
    if (const al::SensorMsgExplosion* pComboMsg =
            sead::DynamicCast<const al::SensorMsgExplosion>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgExplosionCollide* pComboMsg =
            sead::DynamicCast<const al::SensorMsgExplosionCollide>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerKouraAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerKouraAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgKickKouraAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgKickKouraAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgKickKouraAttackCollide* pComboMsg =
            sead::DynamicCast<const al::SensorMsgKickKouraAttackCollide>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackTrample* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackTrample>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackHipDrop* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackHipDrop>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackObjHipDrop* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackObjHipDrop>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackObjHipDropReflect* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackObjHipDropReflect>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackObjHipDropHighJump* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackObjHipDropHighJump>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackStatueDrop* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackStatueDrop>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackObjStatueDrop* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackObjStatueDrop>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackObjStatueDropReflect* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackObjStatueDropReflect>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackInvincibleAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackInvincibleAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerInvincibleTouch* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerInvincibleTouch>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackTailAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackTailAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgBallAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgBallAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgBallTrample* pComboMsg =
            sead::DynamicCast<const al::SensorMsgBallTrample>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackBodyAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackBodyAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackBodyLanding* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackBodyLanding>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackSlidingAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackSlidingAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    // Cooperation hip drops never continue a combo.
    if (sead::IsDerivedFrom<al::SensorMsgPlayerCooperationHipDrop>(pMsg)) {
        return nullptr;
    }

    if (const al::SensorMsgPlayerGiantHipDrop* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerGiantHipDrop>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackGiant* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackGiant>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackClimbAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackClimbAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackClimbSliding* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackClimbSliding>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackBoomerangAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackBoomerangAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgPlayerAttackSpinAttack* pComboMsg =
            sead::DynamicCast<const al::SensorMsgPlayerAttackSpinAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const SensorMsgGorobonAttack* pComboMsg =
            sead::DynamicCast<const SensorMsgGorobonAttack>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const SensorMsgBobsledTrample* pComboMsg =
            sead::DynamicCast<const SensorMsgBobsledTrample>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgBlockUpperPunch* pComboMsg =
            sead::DynamicCast<const al::SensorMsgBlockUpperPunch>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    if (const al::SensorMsgBlockLowerPunch* pComboMsg =
            sead::DynamicCast<const al::SensorMsgBlockLowerPunch>(pMsg)) {
        return pComboMsg->getComboCounter();
    }

    return nullptr;
}

/**
 * Gets the dash time from a DashPanel message.
 * @param pTime Where the dash time is written.
 * @param pMsg The message.
 * @return Whether the message is DashPanel.
 */
bool tryGetDashPanelTime(s32* pTime, const al::SensorMsg* pMsg) {
    const SensorMsgDashPanel* pDashPanelMsg = sead::DynamicCast<const SensorMsgDashPanel>(pMsg);

    if (pDashPanelMsg == nullptr) {
        return false;
    }

    *pTime = pDashPanelMsg->getTime();
    return true;
}

/**
 * Gets the wind power from a ByugoWind or GustWind message.
 * @param pPower Where the wind power is written.
 * @param pMsg The message.
 * @return Whether the message is a wind message.
 */
bool tryGetWindPower(sead::Vector3f* pPower, const al::SensorMsg* pMsg) {
    if (const SensorMsgByugoWind* pByugoMsg = sead::DynamicCast<const SensorMsgByugoWind>(pMsg)) {
        copyVector(pPower, pByugoMsg->getPower());
        return true;
    }

    if (const SensorMsgGustWind* pGustMsg = sead::DynamicCast<const SensorMsgGustWind>(pMsg)) {
        copyVector(pPower, pGustMsg->getPower());
        return true;
    }

    return false;
}

/**
 * Gets the hit direction from a ItemReflect message.
 * @param pHitDir Where the hit direction is written.
 * @param pMsg The message.
 * @return Whether the message is ItemReflect.
 */
bool tryGetItemReflectHitDir(sead::Vector3f* pHitDir, const al::SensorMsg* pMsg) {
    const SensorMsgItemReflect* pItemReflectMsg =
        sead::DynamicCast<const SensorMsgItemReflect>(pMsg);

    if (pItemReflectMsg == nullptr) {
        return false;
    }

    copyVector(pHitDir, pItemReflectMsg->getHitDir());
    return true;
}

/**
 * Gets the hit direction and power from a NeedleRollerHit message.
 * @param pDir Where the hit direction is written.
 * @param pPower Where the hit power is written.
 * @param pMsg The message.
 * @return Whether the message is NeedleRollerHit.
 */
bool tryGetNeedleRollerHitParam(sead::Vector3f* pDir, f32* pPower, const al::SensorMsg* pMsg) {
    const SensorMsgNeedleRollerHit* pHitMsg =
        sead::DynamicCast<const SensorMsgNeedleRollerHit>(pMsg);

    if (pHitMsg == nullptr) {
        return false;
    }

    copyVector(pDir, pHitMsg->getDir());
    *pPower = pHitMsg->getPower();
    return true;
}

/**
 * Gets the front direction from a RouteDokanPlayerTouch message.
 * @param pFront Where the front direction is written.
 * @param pMsg The message.
 * @return Whether the message is RouteDokanPlayerTouch.
 */
bool tryGetRouteDokanPlayerTouchFront(sead::Vector3f* pFront, const al::SensorMsg* pMsg) {
    const SensorMsgRouteDokanPlayerTouch* pRouteDokanPlayerTouchMsg =
        sead::DynamicCast<const SensorMsgRouteDokanPlayerTouch>(pMsg);

    if (pRouteDokanPlayerTouchMsg == nullptr) {
        return false;
    }

    copyVector(pFront, pRouteDokanPlayerTouchMsg->getFront());
    return true;
}

/**
 * Gets the force from a AddForce message.
 * @param pForce Where the force is written.
 * @param pMsg The message.
 * @return Whether the message is AddForce.
 */
bool tryGetForce(sead::Vector3f* pForce, const al::SensorMsg* pMsg) {
    const SensorMsgAddForce* pAddForceMsg = sead::DynamicCast<const SensorMsgAddForce>(pMsg);

    if (pAddForceMsg == nullptr) {
        return false;
    }

    copyVector(pForce, pAddForceMsg->getForce());
    return true;
}

/**
 * Checks whether a QueryHostPlayer message asks about a given player.
 * @param pMsg The message.
 * @param pPlayer The player actor.
 * @return Whether the message is QueryHostPlayer for that player.
 */
bool isEqualHostPlayer(const al::SensorMsg* pMsg, const al::LiveActor* pPlayer) {
    const SensorMsgQueryHostPlayer* pQueryMsg =
        sead::DynamicCast<const SensorMsgQueryHostPlayer>(pMsg);

    if (pQueryMsg == nullptr) {
        return false;
    }

    return pQueryMsg->getHostPlayer() == pPlayer;
}

/**
 * Gets the reaction name from a RequestPlayerGetReaction message.
 * @param pName Where the reaction name is written.
 * @param pMsg The message.
 * @return Whether the message is RequestPlayerGetReaction.
 */
bool tryGetRequestPlayerGetReactionName(const char** pName, const al::SensorMsg* pMsg) {
    const SensorMsgRequestPlayerGetReaction* pRequestPlayerGetReactionMsg =
        sead::DynamicCast<const SensorMsgRequestPlayerGetReaction>(pMsg);

    if (pRequestPlayerGetReactionMsg == nullptr) {
        return false;
    }

    *pName = pRequestPlayerGetReactionMsg->getName();
    return true;
}

/**
 * Forwards a RequestPlayerGetReaction message to another sensor.
 * @param pMsg The message.
 * @param pSender The sending sensor.
 * @param pReceiver The receiving sensor.
 * @return Whether the message is RequestPlayerGetReaction and was forwarded.
 */
bool tryRelayRequestPlayerGetReactionMsg(const al::SensorMsg* pMsg, al::HitSensor* pSender,
                                         al::HitSensor* pReceiver) {
    if (!isMsgRequestPlayerGetReaction(pMsg)) {
        return false;
    }

    sendMsgSensorToSensor(*pMsg, pReceiver, pSender);
    return true;
}

/**
 * Sends a CameraPush message to a player's body sensor.
 * @param pPlayer The player actor.
 * @param pSender The sending sensor.
 * @param rPushVec The push vector.
 * @return Whether the message was received.
 */
bool sendMsgCameraPushToPlayer(al::LiveActor* pPlayer, al::HitSensor* pSender,
                               const sead::Vector3f& rPushVec) {
    return sendMsgCameraPush(al::getHitSensor(pPlayer, "Body"), pSender, rPushVec);
}

/**
 * Starts the blow hit reaction matching a message between two sensors.
 * @param pMsg The message that knocked the actor away.
 * @param pActor The actor that plays the reaction.
 * @param pOther The other sensor.
 * @param pSelf The actor's own sensor.
 */
void startHitReactionBlowHitMessage(const al::SensorMsg* pMsg, const al::LiveActor* pActor,
                                    const al::HitSensor* pOther, const al::HitSensor* pSelf) {
    if (al::isMsgPlayerInvincibleAttack(pMsg) || al::isMsgPlayerInvincibleTouch(pMsg) ||
        al::isMsgPlayerTailAttack(pMsg) || al::isMsgPlayerBoomerangAttack(pMsg) ||
        al::isMsgPlayerBoomerangAttackCollide(pMsg) || al::isMsgPlayerBoomerangReflect(pMsg) ||
        al::isMsgPlayerBoomerangBreak(pMsg)) {
        al::startHitReactionBlowHitDirect(pActor, pOther, pSelf);
    } else {
        al::startHitReactionBlowHit(pActor, pOther, pSelf);
    }
}

/**
 * Starts the blow hit reaction matching a message.
 * @param pMsg The message that knocked the actor away.
 * @param pActor The actor that plays the reaction.
 */
void startHitReactionBlowHitMessage(const al::SensorMsg* pMsg, const al::LiveActor* pActor) {
    if (al::isMsgPlayerInvincibleAttack(pMsg) || al::isMsgPlayerInvincibleTouch(pMsg) ||
        al::isMsgPlayerTailAttack(pMsg) || al::isMsgPlayerBoomerangAttack(pMsg) ||
        al::isMsgPlayerBoomerangAttackCollide(pMsg) || al::isMsgPlayerBoomerangReflect(pMsg) ||
        al::isMsgPlayerBoomerangBreak(pMsg)) {
        al::startHitReactionBlowHitDirect(pActor);
    } else {
        al::startHitReactionBlowHit(pActor);
    }
}

/**
 * Plays the hit reaction matching an attack message on the attacker.
 * @param pMsg The attack message.
 * @param pOther The sensor that was hit.
 * @param pSelf The attacker's sensor.
 */
void requestHitReactionToAttacker(const al::SensorMsg* pMsg, const al::HitSensor* pOther,
                                  const al::HitSensor* pSelf) {
    const al::LiveActor* pHost = al::getSensorHost(pSelf);
    const char* pName;

    if (al::isMsgPlayerTrample(pMsg) || al::isMsgPlayerObjHipDropReflectAll(pMsg) ||
        al::isMsgPlayerObjStatueDropReflectNoCondition(pMsg)) {
        pName = "踏みつけヒット";
    } else if (al::isMsgPlayerHipDropKnockDown(pMsg)) {
        pName = "ヒップドロップ衝撃波";
    } else if (al::isMsgPlayerHipDropAll(pMsg) || al::isMsgPlayerObjHipDropAll(pMsg) ||
               al::isMsgPlayerStatueDrop(pMsg) || al::isMsgPlayerObjStatueDrop(pMsg) ||
               al::isMsgBlockLowerPunch(pMsg)) {
        bool isTrample = al::isSensorNpc(pOther) || al::isSensorKoopaJr(pOther);
        pName = isTrample ? "踏みつけヒット" : "ヒップドロップヒット";
    } else if (al::isMsgPlayerStatueTouch(pMsg)) {
        pName = "像タッチヒット";
    } else if (al::isMsgPlayerRollingAttack(pMsg) || al::isMsgPlayerObjRollingAttack(pMsg) ||
               al::isMsgPlayerObjRollingAttackFailure(pMsg)) {
        pName = "ローリングアタックヒット";
    } else if (al::isMsgPlayerInvincibleAttack(pMsg)) {
        pName = "無敵アタックヒット";
    } else if (al::isMsgPlayerInvincibleTouch(pMsg)) {
        pName = "無敵タッチヒット";
    } else if (al::isMsgPlayerFireBallAttack(pMsg) ||
               al::isMsgPlayerRouteDokanFireBallAttack(pMsg) || al::isMsgEnemyAttackFire(pMsg) ||
               al::isMsgEnemyRouteDokanFire(pMsg)) {
        if (al::isSensorRide(pOther)) {
            return;
        }

        pName = "ファイアボールアタックヒット";
    } else if (al::isMsgPlayerTailAttack(pMsg)) {
        pName = "尻尾アタックヒット";
    } else if (al::isMsgPlayerUpperPunch(pMsg) || al::isMsgPlayerObjUpperPunch(pMsg)) {
        pName = "パンチヒット";
    } else if (al::isMsgPlayerKick(pMsg)) {
        pName = "キックヒット";
    } else if (al::isMsgPlayerSlidingAttack(pMsg)) {
        pName = "スライディングアタックヒット";
    } else if (al::isMsgKickKouraAttack(pMsg) || isMsgRouteDokanKouraAttack(pMsg)) {
        pName = "キック甲羅ヒット";
    } else if (al::isMsgKickKouraReflect(pMsg)) {
        if (al::isSensorRide(pOther)) {
            return;
        }

        pName = "キック甲羅ヒット[反射]";
    } else if (al::isMsgKickKouraBreak(pMsg)) {
        pName = "キック甲羅ヒット[壊れ]";
    } else if (al::isMsgKickStoneAttack(pMsg)) {
        pName = "キック小石ヒット";
    } else if (al::isMsgPlayerBoomerangAttack(pMsg) ||
               al::isMsgPlayerBoomerangAttackCollide(pMsg)) {
        pName = "ブーメランヒット[貫通]";
    } else if (al::isMsgPlayerBoomerangReflect(pMsg)) {
        pName = "ブーメランヒット[反射]";
    } else if (al::isMsgPlayerBoomerangBreak(pMsg)) {
        pName = "ブーメランヒット[壊れ]";
    } else if (al::isMsgBallTrample(pMsg)) {
        pName = "ボール踏みつけヒット";
    } else if (al::isMsgBallAttack(pMsg) || al::isMsgBallRouteDokanAttack(pMsg) ||
               al::isMsgBallAttackHold(pMsg) || al::isMsgBallAttackDRCHold(pMsg) ||
               al::isMsgBallAttackCollide(pMsg) || al::isMsgNekoAttack(pMsg)) {
        pName = "ボール吹き飛ばしヒット";
    } else if (al::isMsgExplosion(pMsg) || al::isMsgExplosionCollide(pMsg) ||
               al::isMsgKillerAttack(pMsg)) {
        pName = "爆発ヒット";
    } else if (al::isMsgPlayerBodyAttack(pMsg) || al::isMsgPlayerBodyAttackReflect(pMsg)) {
        pName = "ボディアタックヒット";
    } else if (al::isMsgPlayerClimbAttack(pMsg)) {
        pName = "クライムアタックヒット";
    } else if (al::isMsgPlayerClimbSlidingAttack(pMsg)) {
        pName = "クライムスライディングアタックヒット";
    } else if (al::isMsgPlayerClimbRollingAttack(pMsg)) {
        pName = "クライムローリングアタックヒット";
    } else if (al::isMsgBlockUpperPunch(pMsg)) {
        pName = "ブロックパンチヒット";
    } else if (al::isMsgPlayerKouraAttack(pMsg)) {
        pName = "キック甲羅ヒット";
    } else if (al::isMsgPlayerCooperationHipDrop(pMsg)) {
        pName = "協力ヒップドロップヒット";
    } else if (al::isMsgPlayerGiantHipDrop(pMsg)) {
        pName = "巨人ヒップドロップヒット";
    } else if (al::isMsgPlayerBodyLanding(pMsg)) {
        pName = "ボディアタックヒット[着地]";
    } else if (al::isMsgPlayerGiantAttack(pMsg)) {
        pName = "巨人アタックヒット";
    } else if (isMsgRouteDokanPlayerAttack(pMsg)) {
        pName = "ルート土管アタックヒット";
    } else if (isMsgSkateShoesAttack(pMsg)) {
        pName = "スケート靴アタックヒット";
    } else if (isMsgBullAttack(pMsg)) {
        pName = "ブルアタックヒット";
    } else if (isMsgJumpPanelAction(pMsg)) {
        pName = "ジャンプパネルヒット";
    } else if (al::isMsgEnemyAttack(pMsg) || isMsgBullAttack(pMsg) || isMsgGorobonAttack(pMsg) ||
               isMsgNeedleRollerAttack(pMsg)) {
        pName = "エネミーアタックヒット";
    } else if (al::isMsgPlayerSpinAttack(pMsg)) {
        pName = "スピンアタックヒット";
    } else if (al::isMsgPlayerGiantTouch(pMsg)) {
        pName = "巨人接触";
    } else if (al::isMsgPlayerObjHipDropHighJump(pMsg)) {
        pName = "ヒップドロップハイジャンプヒット";
    } else if (al::isMsgEnemyAttackBoomerang(pMsg)) {
        pName = "敵飛び道具ヒット";
    } else if (al::isMsgGoalKill(pMsg)) {
        return;
    } else if (isMsgRaidonAttack(pMsg)) {
        pName = "ライドンアタックヒット";
    } else if (isMsgBossGorobonAttack(pMsg) || isMsgBossGorobonSpinShot(pMsg)) {
        pName = "ボスゴロボンアタックヒット";
    } else if (isMsgTuccondorAttack(pMsg)) {
        pName = "ツッコンドルアタックヒット";
    } else if (al::isMsgLaserAttack(pMsg) || al::isMsgBowserPush(pMsg)) {
        return;
    } else if (isMsgBobsledBodyAttack(pMsg) || isMsgBobsledTrample(pMsg)) {
        pName = "HitAttack";
    } else if (al::isMsgDisasterSpikeAttack(pMsg)) {
        return;
    } else if (al::isMsgKeyThrow(pMsg)) {
        pName = "ボール吹き飛ばしヒット";
    } else if (al::isMsgKouraThrow(pMsg) || isMsgPackunEat(pMsg) || isMsgPackunPush(pMsg)) {
        // Known messages without a reaction.
        return;
    } else {
        return;
    }

    al::startHitReactionHitEffect(pHost, pName, pOther, pSelf);
}

/**
 * Plays the NPC hit reaction on the attacker.
 * @param pOther The sensor that was hit.
 * @param pSelf The attacker's sensor.
 */
void requestHitReactionToAttackerNpc(const al::HitSensor* pOther, const al::HitSensor* pSelf) {
    al::startHitReactionHitEffect(al::getSensorHost(pSelf), "ＮＰＣヒット", pOther, pSelf);
}

/**
 * Plays a named hit reaction on the attacker.
 * @param pName The hit reaction name.
 * @param pOther The sensor that was hit.
 * @param pSelf The attacker's sensor.
 */
void requestHitReactionToAttacker(const char* pName, const al::HitSensor* pOther,
                                  const al::HitSensor* pSelf) {
    al::startHitReactionHitEffect(al::getSensorHost(pSelf), pName, pOther, pSelf);
}

/**
 * Sends a DebugMove message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rPos The position to move to.
 * @return Whether the message was received.
 */
bool sendMsgDebugMovePosition(al::HitSensor* pReceiver, al::HitSensor* pSender,
                              const sead::Vector3f& rPos) {
    return sendMsgSensorToSensor(SensorMsgDebugMove(rPos), pReceiver, pSender);
}

/**
 * Gets the position from a DebugMove message.
 * @param pPos Where the position is written.
 * @param pMsg The message.
 * @return Whether the message is DebugMove.
 */
bool tryGetDebugMovePosition(sead::Vector3f* pPos, const al::SensorMsg* pMsg) {
    const SensorMsgDebugMove* pDebugMoveMsg = sead::DynamicCast<const SensorMsgDebugMove>(pMsg);

    if (pDebugMoveMsg == nullptr) {
        return false;
    }

    copyVector(pPos, pDebugMoveMsg->getPos());
    return true;
}

/**
 * Gets the push direction from a PushDir message.
 * @param pDir Where the push direction is written.
 * @param pMsg The message.
 * @return Whether the message is PushDir.
 */
bool tryGetPushDir(sead::Vector3f* pDir, const al::SensorMsg* pMsg) {
    const SensorMsgPushDir* pPushDirMsg = sead::DynamicCast<const SensorMsgPushDir>(pMsg);

    if (pPushDirMsg == nullptr) {
        return false;
    }

    copyVector(pDir, pPushDirMsg->getDir());
    return true;
}

/**
 * Adds velocity along the direction of a PushDir message until a speed is reached.
 * @param pActor The pushed actor.
 * @param pMsg The message.
 * @param speed The speed to reach along the push direction.
 * @return Whether velocity was added.
 */
bool tryReceiveMsgPushDirAndAddVelocity(al::LiveActor* pActor, const al::SensorMsg* pMsg,
                                        f32 speed) {
    if (!isMsgPushDir(pMsg)) {
        return false;
    }

    sead::Vector3f dir;
    tryGetPushDir(&dir, pMsg);
    f32 addSpeed = speed - dir.dot(al::getVelocity(pActor));

    if (addSpeed > 0.0f) {
        sead::Vector3f* pVelocity = al::getVelocityPtr(pActor);
        *pVelocity += dir * addSpeed;
        return true;
    }

    return false;
}

/**
 * Adds velocity away from a connected sensor on a PushConnected message until a speed is reached.
 * @param pActor The pushed actor.
 * @param pMsg The message.
 * @param pOther The pushing sensor.
 * @param pSelf The pushed actor's sensor.
 * @param speed The speed to reach along the push direction.
 * @return Whether velocity was added.
 */
bool tryReceiveMsgPushConnectedAndAddVelocity(al::LiveActor* pActor, const al::SensorMsg* pMsg,
                                              const al::HitSensor* pOther,
                                              const al::HitSensor* pSelf, f32 speed) {
    if (!isMsgPushConnected(pMsg)) {
        return false;
    }

    sead::Vector3f dir = al::getSensorPos(pSelf) - al::getSensorPos(pOther);
    al::normalizeOrDirZ(&dir);
    f32 addSpeed = speed - al::getVelocity(pActor).dot(dir);

    if (addSpeed > 0.0f) {
        sead::Vector3f* pVelocity = al::getVelocityPtr(pActor);
        *pVelocity += dir * addSpeed;
        return true;
    }

    return false;
}

/**
 * Checks whether a message is a JumpPanelAction that triggers a super jump.
 * @param pMsg The message.
 * @return Whether the message is a super jump JumpPanelAction.
 */
bool isMsgJumpPanelActionAndSuperJump(const al::SensorMsg* pMsg) {
    if (!isMsgJumpPanelAction(pMsg)) {
        return false;
    }

    const SensorMsgJumpPanelAction* pJumpMsg =
        sead::DynamicCast<const SensorMsgJumpPanelAction>(pMsg);

    if (pJumpMsg != nullptr && pJumpMsg->getSuperJump()) {
        return true;
    }

    return false;
}

}  // namespace rc
