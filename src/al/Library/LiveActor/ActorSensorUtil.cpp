#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

#include "Library/Actor/ActorSensorController.hpp"
#include "Library/Actor/ComboCounter.hpp"
#include "Library/HitSensor/HitSensor.hpp"
#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/HitSensor/HitSensorDirector.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/HitSensor/SensorMsg.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
SENSOR_MSG_WITH_DATA(PlayerAttackTrample, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDropReflect, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjHipDropHighJump, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerAttackHipDropKnockDown);
SENSOR_MSG_WITH_DATA(PlayerAttackStatueDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjStatueDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackObjStatueDropReflect, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerAttackObjStatueDropReflectNoCondition);
SENSOR_MSG(PlayerAttackStatueTouch);
SENSOR_MSG(PlayerAttackUpperPunch);
SENSOR_MSG(PlayerAttackObjUpperPunch);
SENSOR_MSG(PlayerAttackRollingAttack);
SENSOR_MSG(PlayerAttackRollingReflect);
SENSOR_MSG(PlayerAttackObjRollingAttack);
SENSOR_MSG(PlayerAttackObjRollingAttackFailure);
SENSOR_MSG_WITH_DATA(PlayerAttackInvincibleAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackInvincibleHelpAttack, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerAttackFireBallAttack);
SENSOR_MSG(PlayerAttackRouteDokanFireBallAttack);
SENSOR_MSG_WITH_DATA(PlayerAttackTailAttack, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerAttackKick);
SENSOR_MSG(PlayerAttackCatch);
SENSOR_MSG_WITH_DATA(PlayerAttackSlidingAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBoomerangAttack, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerAttackBoomerangAttackCollide);
SENSOR_MSG(PlayerAttackBoomerangReflect);
SENSOR_MSG(PlayerAttackBoomerangBreak);
SENSOR_MSG_WITH_DATA(PlayerAttackBodyAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBodyLanding, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackBodyAttackReflect, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackClimbAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackSpinAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackGiant, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerCooperationHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackClimbSliding, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerAttackClimbRolling, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(PlayerGiantHipDrop, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerGiantTouch);
SENSOR_MSG(PlayerDisregard);
SENSOR_MSG(PlayerItemGet);
SENSOR_MSG(KeyOpen);
SENSOR_MSG(KeyThrow);
SENSOR_MSG(PlayerReleaseEquipment);
SENSOR_MSG_WITH_DATA(PlayerReleaseEquipmentGoal, u32, Type);
SENSOR_MSG(PlayerFloorTouch);
SENSOR_MSG(KoopaJrFloorTouch);
SENSOR_MSG(PlayerDamageTouch);
SENSOR_MSG(PlayerCarryFront);
SENSOR_MSG(PlayerCarryFrontNeko);
SENSOR_MSG(PlayerCarryUp);
SENSOR_MSG(PlayerCarryUpTest);
SENSOR_MSG(PlayerCanCarry);
SENSOR_MSG(PlayerLeave);
SENSOR_MSG(PlayerRelease);
SENSOR_MSG(PlayerReleaseDamage);
SENSOR_MSG(PlayerReleaseDead);
SENSOR_MSG(PlayerToss);
SENSOR_MSG_WITH_DATA(PlayerInvincibleTouch, ComboCounter*, ComboCounter);
SENSOR_MSG(PlayerHideItem);
SENSOR_MSG(PlayerShowItem);
SENSOR_MSG(PressureDeath);
SENSOR_MSG(NpcTouch);
SENSOR_MSG(EnemyAttack);
SENSOR_MSG(EnemyAttackFire);
SENSOR_MSG(EnemyAttackBoomerang);
SENSOR_MSG(EnemyRouteDokanAttack);
SENSOR_MSG(EnemyRouteDokanFire);
SENSOR_MSG_WITH_DATA(Explosion, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(ExplosionCollide, ComboCounter*, ComboCounter);
SENSOR_MSG(Push);
SENSOR_MSG(PushStrong);
SENSOR_MSG(PushVeryStrong);
SENSOR_MSG(Hit);
SENSOR_MSG(HitStrong);
SENSOR_MSG(HitVeryStrong);
SENSOR_MSG(KnockDown);
SENSOR_MSG(Vanish);
SENSOR_MSG(ShowModel);
SENSOR_MSG(HideModel);
SENSOR_MSG(EnemyTouch);
SENSOR_MSG(EnemyFloorTouch);
SENSOR_MSG(EnemyUpperPunch);
SENSOR_MSG(PunpunFloorTouch);
SENSOR_MSG(InvalidateFootPrint);
SENSOR_MSG_WITH_DATA(KickKouraAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(KickKouraAttackCollide, ComboCounter*, ComboCounter);
SENSOR_MSG(KickKouraGetItem);
SENSOR_MSG(KickKouraReflect);
SENSOR_MSG(KickKouraCollideNoReflect);
SENSOR_MSG(KickKouraBreak);
SENSOR_MSG(KickKouraBlow);
SENSOR_MSG(KouraDestroy);
SENSOR_MSG(KouraThrow);
SENSOR_MSG(KickStoneAttack);
SENSOR_MSG(KillerAttack);
SENSOR_MSG(KillerReflect);
SENSOR_MSG(LiftGeyser);
SENSOR_MSG(WarpStart);
SENSOR_MSG(WarpEnd);
SENSOR_MSG(HoldCancel);
SENSOR_MSG(HoldCancelWarp);
SENSOR_MSG(HoleIn);
SENSOR_MSG(JumpInhibit);
SENSOR_MSG(GoalKill);
SENSOR_MSG(Goal);
SENSOR_MSG(BindStart);
SENSOR_MSG_WITH_DATA(BindInit, u32, Type);
SENSOR_MSG(BindEnd);
SENSOR_MSG(BindCancel);
SENSOR_MSG(BindCancelForGoal);
SENSOR_MSG(BindCancelForWarp);
SENSOR_MSG(BindCancelForDemo);
SENSOR_MSG(BindDamage);
SENSOR_MSG(BindSteal);
SENSOR_MSG(BindGiant);
SENSOR_MSG_WITH_DATA(BallAttack, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BallRouteDokanAttack, ComboCounter*, ComboCounter);
SENSOR_MSG(BallAttackHold);
SENSOR_MSG(BallAttackDRCHold);
SENSOR_MSG(BallAttackCollide);
SENSOR_MSG_WITH_DATA(BallTrample, ComboCounter*, ComboCounter);
SENSOR_MSG(BallTrampleCollide);
SENSOR_MSG(BallItemGet);
SENSOR_MSG(FireBallCollide);
SENSOR_MSG(FireBallFloorTouch);
SENSOR_MSG(DokanBazookaAttack);
SENSOR_MSG(RideAllPlayerItemGet);
SENSOR_MSG(AskSafetyPoint);
SENSOR_MSG(TouchAssist);
SENSOR_MSG(TouchAssistTrig);
SENSOR_MSG(StrokeTransparent);
SENSOR_MSG(ScreenPointInvalidCollisionParts);
SENSOR_MSG_WITH_DATA(BlockUpperPunch, ComboCounter*, ComboCounter);
SENSOR_MSG_WITH_DATA(BlockLowerPunch, ComboCounter*, ComboCounter);
SENSOR_MSG(BlockItemGet);
SENSOR_MSG(KillerItemGet);
SENSOR_MSG_WITH_DATA(PlayerKouraAttack, ComboCounter*, ComboCounter);
SENSOR_MSG(LightFlash);
SENSOR_MSG(HeadlightFlash);
SENSOR_MSG(ForceAbyss);
SENSOR_MSG(IsNerveSupportFreeze);
SENSOR_MSG(OnSyncSupportFreeze);
SENSOR_MSG(OffSyncSupportFreeze);
SENSOR_MSG(SwordAttackHighLeft);
SENSOR_MSG(SwordAttackLowLeft);
SENSOR_MSG(SwordAttackHighRight);
SENSOR_MSG(SwordAttackLowRight);
SENSOR_MSG(SwordAttackJumpUnder);
SENSOR_MSG(SwordBeamAttack);
SENSOR_MSG(SwordBeamReflectAttack);
SENSOR_MSG(ShieldGuard);
SENSOR_MSG(EnemyAttackKnockDown);
SENSOR_MSG(AskMultiPlayerEnemy);
SENSOR_MSG(ItemGettable);
SENSOR_MSG(KikkiThrow);
SENSOR_MSG(IsKikkiThrowTarget);
SENSOR_MSG(PlayerCloudGet);
SENSOR_MSG(AutoJump);
SENSOR_MSG(Sink);
SENSOR_MSG(LaserAttack);
SENSOR_MSG(GigaStomp);
SENSOR_MSG(Restore);
SENSOR_MSG(GigaEnemyAttack);
SENSOR_MSG(NekoAttack);
SENSOR_MSG(NekoPush);
SENSOR_MSG(DisasterSpikeAttack);
SENSOR_MSG(DisasterSpikePush);
SENSOR_MSG(PlessieFloorTouch);
SENSOR_MSG(GigaBellPush);
SENSOR_MSG(BowserPush);
SENSOR_MSG(CutsceneStart);
SENSOR_MSG(IsItemHomingTarget);
SENSOR_MSG(PlayerAttackDash);
SENSOR_MSG(PlayerFloorTouchBind);
SENSOR_MSG(PlayerTouch);
SENSOR_MSG(TouchAssistNoPat);
SENSOR_MSG(TouchAssistTrigNoPat);
SENSOR_MSG(TouchAssistBurn);
SENSOR_MSG(TouchCarryItem);
SENSOR_MSG(TouchReleaseItem);

static inline bool sendMsgSensorToSensor(const SensorMsg& rMsg, HitSensor* pReceiver,
                                         HitSensor* pSender) {
    return pReceiver->mHostActor->receiveMsg(&rMsg, pSender, pReceiver);
}

static bool isCrossoverSensor(const HitSensor* pSelf, const HitSensor* pOther);

static inline bool isInsideBoxWithRadius(const sead::BoundBox3f& rBox, const sead::Vector3f& rPos,
                                         f32 radius) {
    sead::Vector3f r(radius, radius, radius);
    sead::BoundBox3f box(rBox.getMin() - r, r + rBox.getMax());
    return box.isInside(rPos);
}

/**
 * Adds a Player hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorPlayer(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::Player), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a hit sensor following the actor's translation and registers it in its hit group.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param type The sensor type.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensor(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, u32 type,
                        f32 radius, u16 maxSensors, const sead::Vector3f& rOffset) {
    HitSensor* sensor = pActor->mHitSensorKeeper->addSensor(pActor, pName, type, radius, maxSensors,
                                                            getTransPtr(pActor), nullptr, rOffset);
    rInfo.mHitSensorDirector->initGroup(sensor);
    return sensor;
}

/**
 * Adds a PlayerEye hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorPlayerEye(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::PlayerEye), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a Enemy hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorEnemy(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::Enemy), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a MapObj hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorMapObj(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::MapObj), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a Bindable hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindable(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::Bindable), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableGoal hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableGoal(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableGoal), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableAllPlayer hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableAllPlayer(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableAllPlayer), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableBubbleOutScreen hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableBubbleOutScreen(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableBubbleOutScreen), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableKoura hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableKoura(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableKoura), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableRouteDokan hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableRouteDokan(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableRouteDokan), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a BindableBubblePadInput hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorBindableBubblePadInput(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::BindableBubblePadInput), radius, maxSensors,
                        rOffset);
}

/**
 * Adds a Eye hit sensor following the actor's translation.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pName The sensor name.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param rOffset The offset from the actor's translation.
 * @return The new sensor.
 */
HitSensor* addHitSensorEye(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName, f32 radius,
                u16 maxSensors, const sead::Vector3f& rOffset) {
    return addHitSensor(pActor, rInfo, pName, static_cast<u32>(HitSensorType::Eye), radius, maxSensors,
                        rOffset);
}

/**
 * Gets the number of hit sensors of an actor.
 * @param pActor The actor.
 * @return The number of sensors.
 */
s32 getHitSensorNum(const LiveActor* pActor) {
    return pActor->mHitSensorKeeper->mSensorCount;
}

/**
 * Sets the function used to sort the sensors hit by a sensor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param pFunc The compare function.
 */
void setHitSensorSort(LiveActor* pActor, const char* pName, const SensorSortCmpFuncBase* pFunc) {
    HitSensor* sensor = pActor->mHitSensorKeeper->getSensor(pName);
    sensor->mSortFunc = new SensorSortCmpFunc(pFunc);
}

/**
 * Makes a sensor follow a position.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param pPos The position to follow.
 */
void setHitSensorPosPtr(LiveActor* pActor, const char* pName, const sead::Vector3f* pPos) {
    pActor->mHitSensorKeeper->getSensor(pName)->setFollowPosPtr(pPos);
}

/**
 * Gets a hit sensor by name.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @return The sensor.
 */
HitSensor* getHitSensor(const LiveActor* pActor, const char* pName) {
    return pActor->mHitSensorKeeper->getSensor(pName);
}

/**
 * Makes a sensor follow a matrix.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param pMtx The matrix to follow.
 */
void setHitSensorMtxPtr(LiveActor* pActor, const char* pName, const sead::Matrix34f* pMtx) {
    pActor->mHitSensorKeeper->getSensor(pName)->setFollowMtxPtr(pMtx);
}

/**
 * Makes a sensor follow a joint matrix of the actor's model.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param pJointName The joint name.
 */
void setHitSensorJointMtx(LiveActor* pActor, const char* pName, const char* pJointName) {
    HitSensor* sensor = pActor->mHitSensorKeeper->getSensor(pName);
    sensor->setFollowMtxPtr(getJointMtxPtr(pActor, pJointName));
}

/**
 * Gets the matrix a sensor follows.
 * @param pSensor The sensor.
 * @return The followed matrix.
 */
const sead::Matrix34f* getHitSensorFollowMtx(HitSensor* pSensor) {
    return pSensor->mFollowMtx;
}

/**
 * Gets a hit sensor by index.
 * @param pActor The actor.
 * @param idx The sensor index.
 * @return The sensor.
 */
HitSensor* getHitSensor(const LiveActor* pActor, s32 idx) {
    return pActor->mHitSensorKeeper->getSensor(idx);
}

/**
 * Sets the radius of a sensor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param radius The new radius.
 */
void setSensorRadius(LiveActor* pActor, const char* pName, f32 radius) {
    pActor->mHitSensorKeeper->getSensor(pName)->mRadius = radius;
}

/**
 * Sets the radius of a sensor.
 * @param pActor The actor.
 * @param idx The sensor index.
 * @param radius The new radius.
 */
void setSensorRadius(LiveActor* pActor, s32 idx, f32 radius) {
    pActor->mHitSensorKeeper->getSensor(idx)->mRadius = radius;
}

/**
 * Sets the radius of the actor's only sensor.
 * @param pActor The actor.
 * @param radius The new radius.
 */
void setSensorRadius(LiveActor* pActor, f32 radius) {
    pActor->mHitSensorKeeper->getSensor(static_cast<const char*>(nullptr))->mRadius = radius;
}

/**
 * Gets the radius of a sensor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @return The radius.
 */
f32 getSensorRadius(const LiveActor* pActor, const char* pName) {
    return pActor->mHitSensorKeeper->getSensor(pName)->mRadius;
}

/**
 * Gets the radius of a sensor.
 * @param pActor The actor.
 * @param idx The sensor index.
 * @return The radius.
 */
f32 getSensorRadius(const LiveActor* pActor, s32 idx) {
    return pActor->mHitSensorKeeper->getSensor(idx)->mRadius;
}

/**
 * Gets the radius of the actor's only sensor.
 * @param pActor The actor.
 * @return The radius.
 */
f32 getSensorRadius(const LiveActor* pActor) {
    return pActor->mHitSensorKeeper->getSensor(static_cast<const char*>(nullptr))->mRadius;
}

/**
 * Sets the follow offset of a sensor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @param rOffset The new offset.
 */
void setSensorFollowPosOffset(LiveActor* pActor, const char* pName, const sead::Vector3f& rOffset) {
    HitSensor* sensor = pActor->mHitSensorKeeper->getSensor(pName);
    sensor->mFollowPosOffset.e = rOffset.e;
}

/**
 * Sets the follow offset of a sensor.
 * @param pActor The actor.
 * @param idx The sensor index.
 * @param rOffset The new offset.
 */
void setSensorFollowPosOffset(LiveActor* pActor, s32 idx, const sead::Vector3f& rOffset) {
    HitSensor* sensor = pActor->mHitSensorKeeper->getSensor(idx);
    sensor->mFollowPosOffset.e = rOffset.e;
}

/**
 * Sets the follow offset of the actor's only sensor.
 * @param pActor The actor.
 * @param rOffset The new offset.
 */
void setSensorFollowPosOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {
    HitSensor* sensor = pActor->mHitSensorKeeper->getSensor(static_cast<const char*>(nullptr));
    sensor->mFollowPosOffset.e = rOffset.e;
}

/**
 * Gets the follow offset of a sensor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @return The offset.
 */
const sead::Vector3f& getSensorFollowPosOffset(const LiveActor* pActor, const char* pName) {
    return pActor->mHitSensorKeeper->getSensor(pName)->mFollowPosOffset;
}

/**
 * Gets the follow offset of a sensor.
 * @param pActor The actor.
 * @param idx The sensor index.
 * @return The offset.
 */
const sead::Vector3f& getSensorFollowPosOffset(const LiveActor* pActor, s32 idx) {
    return pActor->mHitSensorKeeper->getSensor(idx)->mFollowPosOffset;
}

/**
 * Gets the follow offset of the actor's only sensor.
 * @param pActor The actor.
 * @return The offset.
 */
const sead::Vector3f& getSensorFollowPosOffset(const LiveActor* pActor) {
    return pActor->mHitSensorKeeper->getSensor(static_cast<const char*>(nullptr))->mFollowPosOffset;
}

/**
 * Creates a sensor controller for a sensor of an actor.
 * @param pActor The actor.
 * @param pName The sensor name.
 * @return The new controller.
 */
ActorSensorController* createActorSensorController(LiveActor* pActor, const char* pName) {
    return new ActorSensorController(pActor, pName);
}

/**
 * Sets the radius of a controlled sensor.
 * @param pController The sensor controller.
 * @param radius The new radius.
 */
void setSensorRadius(ActorSensorController* pController, f32 radius) {
    pController->setSensorRadius(radius);
}

/**
 * Sets the follow offset of a controlled sensor.
 * @param pController The sensor controller.
 * @param rOffset The new offset.
 */
void setSensorFollowPosOffset(ActorSensorController* pController, const sead::Vector3f& rOffset) {
    pController->setSensorFollowPosOffset(rOffset);
}

/**
 * Gets the original radius of a controlled sensor.
 * @param pController The sensor controller.
 * @return The original radius.
 */
f32 getOriginalSensorRadius(const ActorSensorController* pController) {
    return pController->mSensorRadius;
}

/**
 * Gets the original follow offset of a controlled sensor.
 * @param pController The sensor controller.
 * @return The original offset.
 */
const sead::Vector3f& getOriginalSensorFollowPosOffset(const ActorSensorController* pController) {
    return pController->mFollowPosOffs;
}

/**
 * Resets a controlled sensor to its original radius and offset.
 * @param pController The sensor controller.
 */
void resetActorSensorController(ActorSensorController* pController) {
    pController->resetActorSensorController();
}

/**
 * Calculates the point between the surfaces of two sensors, offset from the first sensor.
 * @param pOut The resulting position.
 * @param pA The first sensor.
 * @param pB The second sensor.
 * @param offset The extra distance from the first sensor's surface.
 */
void calcPosBetweenSensors(sead::Vector3f* pOut, const HitSensor* pA, const HitSensor* pB,
                           f32 offset) {
    sead::Vector3f dir = pB->mPos;
    dir -= pA->mPos;
    if (isNearZero(dir, 0.001f)) {
        pOut->e = pA->mPos.e;
        return;
    }

    normalize(&dir);
    f32 gap = (pA->mPos - pB->mPos).length() - (pA->mRadius + pB->mRadius);
    pOut->e = dir.e;
    *pOut *= pA->mRadius + gap * 0.5f + offset;
    *pOut += pA->mPos;
}

/**
 * Calculates the distance between two sensors projected on a direction.
 * @param rDir The direction.
 * @param pA The first sensor.
 * @param pB The second sensor.
 * @return The projected distance.
 */
f32 calcDistanceV(const sead::Vector3f& rDir, const HitSensor* pA, const HitSensor* pB) {
    sead::Vector3f diff = pB->mPos;
    diff -= pA->mPos;
    if (isNearZero(diff, 0.001f)) {
        return diff.length();
    }

    sead::Vector3f dir = diff;
    normalize(&dir);
    return dir.dot(rDir) * diff.length();
}

/**
 * Calculates the normalized direction from one sensor to another.
 * @param pOut The resulting direction.
 * @param pFrom The start sensor.
 * @param pTo The end sensor.
 */
void calcDirBetweenSensors(sead::Vector3f* pOut, const HitSensor* pFrom, const HitSensor* pTo) {
    pOut->setSub(pTo->mPos, pFrom->mPos);
    normalizeOrZero(pOut);
}

/**
 * Calculates the normalized horizontal direction from one sensor to another.
 * @param pOut The resulting direction.
 * @param pFrom The start sensor.
 * @param pTo The end sensor.
 */
void calcDirBetweenSensorsH(sead::Vector3f* pOut, const HitSensor* pFrom, const HitSensor* pTo) {
    pOut->setSub(pTo->mPos, pFrom->mPos);
    pOut->y = 0.0f;
    normalizeOrZero(pOut);
}

/**
 * Calculates the vector from one sensor to another.
 * @param pOut The resulting vector.
 * @param pFrom The start sensor.
 * @param pTo The end sensor.
 */
void calcVecBetweenSensors(sead::Vector3f* pOut, const HitSensor* pFrom, const HitSensor* pTo) {
    pOut->setSub(pTo->mPos, pFrom->mPos);
}

/**
 * Calculates the horizontal vector from one sensor to another.
 * @param pOut The resulting vector.
 * @param pFrom The start sensor.
 * @param pTo The end sensor.
 */
void calcVecBetweenSensorsH(sead::Vector3f* pOut, const HitSensor* pFrom, const HitSensor* pTo) {
    pOut->setSub(pTo->mPos, pFrom->mPos);
    pOut->y = 0.0f;
}

/**
 * Checks whether a sensor touches a box placed at a position.
 * @param pSensor The sensor.
 * @param rPos The box position.
 * @param rBox The box.
 * @return Whether the sensor touches the box.
 */
bool isHitBoxSensor(const HitSensor* pSensor, const sead::Vector3f& rPos,
                    const sead::BoundBox3f& rBox) {
    sead::Vector3f local = pSensor->mPos - rPos;
    return isInsideBoxWithRadius(rBox, local, pSensor->mRadius);
}

/**
 * Gets the radius of a sensor.
 * @param pSensor The sensor.
 * @return The radius.
 */
f32 getSensorRadius(const HitSensor* pSensor) {
    return pSensor->mRadius;
}

/**
 * Gets the position of a sensor.
 * @param pSensor The sensor.
 * @return The position.
 */
const sead::Vector3f& getSensorPos(const HitSensor* pSensor) {
    return pSensor->mPos;
}

/**
 * Checks whether a sensor touches a box transformed by a matrix.
 * @param pSensor The sensor.
 * @param rMtx The box transform.
 * @param rBox The box.
 * @return Whether the sensor touches the box.
 */
bool isHitBoxSensor(const HitSensor* pSensor, const sead::Matrix34f& rMtx,
                    const sead::BoundBox3f& rBox) {
    sead::Matrix34f inv;
    inv.setInverse(rMtx);
    sead::Vector3f local;
    local.setMul(inv, pSensor->mPos);
    return isInsideBoxWithRadius(rBox, local, pSensor->mRadius);
}

/**
 * Checks whether a sensor touches an infinite cylinder.
 * @param pSensor The sensor.
 * @param rPos A point on the cylinder axis.
 * @param rAxis The cylinder axis.
 * @param radius The cylinder radius.
 * @return Whether the sensor touches the cylinder.
 */
bool isHitCylinderSensor(const HitSensor* pSensor, const sead::Vector3f& rPos,
                         const sead::Vector3f& rAxis, f32 radius) {
    sead::Vector3f vertical;
    sead::Vector3f diff = rPos - pSensor->mPos;
    verticalizeVec(&vertical, rAxis, diff);
    return vertical.length() <= pSensor->mRadius + radius;
}

/**
 * Checks whether a sensor touches an infinite cylinder going through another sensor.
 * @param pSensor The sensor.
 * @param pOther The sensor on the cylinder axis.
 * @param rAxis The cylinder axis.
 * @param radius The cylinder radius.
 * @return Whether the sensor touches the cylinder.
 */
bool isHitCylinderSensor(const HitSensor* pSensor, const HitSensor* pOther,
                         const sead::Vector3f& rAxis, f32 radius) {
    return isHitCylinderSensor(pSensor, pOther->mPos, rAxis, radius);
}

/**
 * Checks whether a sensor touches an infinite cylinder and calculates the hit position and normal.
 * @param pHitPos The hit position, or nullptr.
 * @param pHitNormal The hit normal, or nullptr.
 * @param pSensor The sensor.
 * @param rPos A point on the cylinder axis.
 * @param rAxis The cylinder axis.
 * @param radius The cylinder radius.
 * @return Whether the sensor touches the cylinder.
 */
bool isHitCylinderSensor(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal,
                         const HitSensor* pSensor, const sead::Vector3f& rPos,
                         const sead::Vector3f& rAxis, f32 radius) {
    sead::Vector3f vertical;
    sead::Vector3f diff = pSensor->mPos - rPos;
    verticalizeVec(&vertical, rAxis, diff);
    f32 dist = vertical.length();
    if (dist > pSensor->mRadius + radius) {
        return false;
    }

    if (pHitPos != nullptr || pHitNormal != nullptr) {
        if (isNearZero(dist, 0.001f)) {
            if (pHitNormal != nullptr) {
                calcDirVerticalAny(pHitNormal, rAxis);
            }

            if (pHitPos != nullptr) {
                pHitPos->e = pSensor->mPos.e;
            }
        } else {
            sead::Vector3f dir = vertical * (1.0f / dist);
            if (pHitNormal != nullptr) {
                *pHitNormal = dir;
            }

            if (pHitPos != nullptr) {
                *pHitPos = (pSensor->mPos - vertical) + dir * (dist + (radius - pSensor->mRadius));
            }
        }
    }

    return true;
}

/**
 * Checks whether a sensor touches an infinite cylinder going through another sensor.
 * @param pHitPos The hit position, or nullptr.
 * @param pHitNormal The hit normal, or nullptr.
 * @param pSensor The sensor.
 * @param pOther The sensor on the cylinder axis.
 * @param rAxis The cylinder axis.
 * @param radius The cylinder radius.
 * @return Whether the sensor touches the cylinder.
 */
bool isHitCylinderSensor(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal,
                         const HitSensor* pSensor, const HitSensor* pOther,
                         const sead::Vector3f& rAxis, f32 radius) {
    return isHitCylinderSensor(pHitPos, pHitNormal, pSensor, pOther->mPos, rAxis, radius);
}

/**
 * Checks whether a sensor touches a circle and calculates the hit position and normal.
 * @param pHitPos The hit position, or nullptr.
 * @param pHitNormal The hit normal, or nullptr.
 * @param pSensor The sensor.
 * @param rCenter The circle center.
 * @param rNormal The circle normal.
 * @param circleRadius The circle radius.
 * @param width The circle thickness.
 * @return Whether the sensor touches the circle.
 */
bool isHitCircleSensor(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal,
                       const HitSensor* pSensor, const sead::Vector3f& rCenter,
                       const sead::Vector3f& rNormal, f32 circleRadius, f32 width) {
    sead::Vector3f vertical;
    sead::Vector3f diff = pSensor->mPos - rCenter;
    verticalizeVec(&vertical, rNormal, diff);
    if (normalizeOrZero(&vertical)) {
        calcDirVerticalAny(&vertical, rNormal);
    }

    sead::Vector3f edge = vertical * circleRadius + rCenter;
    diff = pSensor->mPos - edge;
    f32 dist = diff.length();
    if (dist > pSensor->mRadius + width) {
        return false;
    }

    if (pHitPos != nullptr || pHitNormal != nullptr) {
        if (normalizeOrZero(&diff)) {
            if (pHitNormal != nullptr) {
                pHitNormal->e = vertical.e;
            }

            if (pHitPos != nullptr) {
                *pHitPos = edge;
            }
        } else {
            if (pHitNormal != nullptr) {
                pHitNormal->e = diff.e;
            }

            if (pHitPos != nullptr) {
                *pHitPos = edge + diff * (dist + (width - pSensor->mRadius));
            }
        }
    }

    return true;
}

/**
 * Checks whether a sensor touches a circle centered on another sensor.
 * @param pHitPos The hit position, or nullptr.
 * @param pHitNormal The hit normal, or nullptr.
 * @param pSensor The sensor.
 * @param pCenter The sensor at the circle center.
 * @param rNormal The circle normal.
 * @param circleRadius The circle radius.
 * @param width The circle thickness.
 * @return Whether the sensor touches the circle.
 */
bool isHitCircleSensor(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal,
                       const HitSensor* pSensor, const HitSensor* pCenter,
                       const sead::Vector3f& rNormal, f32 circleRadius, f32 width) {
    return isHitCircleSensor(pHitPos, pHitNormal, pSensor, pCenter->mPos, rNormal, circleRadius,
                             width);
}

/**
 * Checks whether a sensor touches a circle.
 * @param pSensor The sensor.
 * @param rCenter The circle center.
 * @param rNormal The circle normal.
 * @param circleRadius The circle radius.
 * @param width The circle thickness.
 * @return Whether the sensor touches the circle.
 */
bool isHitCircleSensor(const HitSensor* pSensor, const sead::Vector3f& rCenter,
                       const sead::Vector3f& rNormal, f32 circleRadius, f32 width) {
    sead::Vector3f vertical;
    sead::Vector3f diff = pSensor->mPos - rCenter;
    verticalizeVec(&vertical, rNormal, diff);
    if (normalizeOrZero(&vertical)) {
        f32 height = (pSensor->mPos - rCenter).dot(rNormal);
        return sead::Mathf::sqrt(circleRadius * circleRadius + height * height) <=
               pSensor->mRadius + width;
    }

    sead::Vector3f edge = vertical * circleRadius + rCenter;
    return (pSensor->mPos - edge).length() <= pSensor->mRadius + width;
}

/**
 * Checks whether a sensor touches a circle centered on another sensor.
 * @param pSensor The sensor.
 * @param pCenter The sensor at the circle center.
 * @param rNormal The circle normal.
 * @param circleRadius The circle radius.
 * @param width The circle thickness.
 * @return Whether the sensor touches the circle.
 */
bool isHitCircleSensor(const HitSensor* pSensor, const HitSensor* pCenter,
                       const sead::Vector3f& rNormal, f32 circleRadius, f32 width) {
    return isHitCircleSensor(pSensor, pCenter->mPos, rNormal, circleRadius, width);
}

/**
 * Gets the actor owning a sensor.
 * @param pSensor The sensor.
 * @return The host actor.
 */
LiveActor* getSensorHost(const HitSensor* pSensor) {
    return pSensor->mHostActor;
}

/**
 * Gets the translation of the actor owning a sensor.
 * @param pSensor The sensor.
 * @return The host's translation.
 */
const sead::Vector3f& getActorTrans(const HitSensor* pSensor) {
    return getTrans(pSensor->mHostActor);
}

/**
 * Gets the velocity of the actor owning a sensor.
 * @param pSensor The sensor.
 * @return The host's velocity.
 */
const sead::Vector3f& getActorVelocity(const HitSensor* pSensor) {
    return getVelocity(pSensor->mHostActor);
}

/**
 * Checks the name of a sensor.
 * @param pSensor The sensor.
 * @param pName The name to compare with.
 * @return Whether the sensor has that name.
 */
bool isSensorName(const HitSensor* pSensor, const char* pName) {
    return isEqualString(pSensor->mName, pName);
}

/**
 * Checks the name of the actor owning a sensor.
 * @param pSensor The sensor.
 * @param pName The name to compare with.
 * @return Whether the host has that name.
 */
bool isSensorHostName(const HitSensor* pSensor, const char* pName) {
    return isEqualString(pSensor->mHostActor->getName(), pName);
}

/**
 * Checks whether the name of the actor owning a sensor contains a string.
 * @param pSensor The sensor.
 * @param pName The string to search for.
 * @return Whether the host's name contains the string.
 */
bool isSensorHostSubName(const HitSensor* pSensor, const char* pName) {
    return searchSubString(pSensor->mHostActor->getName(), pName) != nullptr;
}

/**
 * Validates all sensors of an actor, if it has any.
 * @param pActor The actor.
 */
void validateHitSensors(LiveActor* pActor) {
    if (pActor->mHitSensorKeeper != nullptr) {
        pActor->mHitSensorKeeper->validate();
    }
}

/**
 * Invalidates all sensors of an actor, if it has any.
 * @param pActor The actor.
 */
void invalidateHitSensors(LiveActor* pActor) {
    if (pActor->mHitSensorKeeper != nullptr) {
        pActor->mHitSensorKeeper->invalidate();
    }
}

/**
 * Checks whether a sensor is valid and validated by the system.
 * @param pSensor The sensor.
 * @return Whether the sensor is valid.
 */
bool isSensorValid(const HitSensor* pSensor) {
    return pSensor->mIsValid && pSensor->mIsValidBySystem;
}

/**
 * Validates a sensor of an actor.
 * @param pActor The actor.
 * @param pName The sensor name.
 */
void validateHitSensor(LiveActor* pActor, const char* pName) {
    pActor->mHitSensorKeeper->getSensor(pName)->validate();
}

/**
 * Invalidates a sensor of an actor.
 * @param pActor The actor.
 * @param pName The sensor name.
 */
void invalidateHitSensor(LiveActor* pActor, const char* pName) {
    pActor->mHitSensorKeeper->getSensor(pName)->invalidate();
}

/**
 * Sends a PlayerAttackTrample message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerAttackTrample(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackTrample(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackHipDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerHipDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackHipDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjHipDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjHipDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjHipDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjHipDropReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjHipDropReflect(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjHipDropReflect(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjHipDropHighJump message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjHipDropHighJump(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjHipDropHighJump(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackHipDropKnockDown message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerHipDropKnockDown(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackHipDropKnockDown(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackStatueDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerStatueDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackStatueDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjStatueDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjStatueDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjStatueDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjStatueDropReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjStatueDropReflect(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjStatueDropReflect(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjStatueDropReflectNoCondition message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjStatueDropReflectNoCondition(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjStatueDropReflectNoCondition(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackStatueTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerStatueTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackStatueTouch(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackUpperPunch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerUpperPunch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackUpperPunch(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjUpperPunch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjUpperPunch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjUpperPunch(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackRollingAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerRollingAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackRollingAttack(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackRollingReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerRollingReflect(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackRollingReflect(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjRollingAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjRollingAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjRollingAttack(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackObjRollingAttackFailure message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerObjRollingAttackFailure(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackObjRollingAttackFailure(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackInvincibleAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerInvincibleAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackInvincibleAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackInvincibleHelpAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerInvincibleHelpAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackInvincibleHelpAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackFireBallAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerFireBallAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackFireBallAttack(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackRouteDokanFireBallAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerRouteDokanFireBallAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackRouteDokanFireBallAttack(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackTailAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerTailAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackTailAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackKick message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerKick(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackKick(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackCatch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCatch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackCatch(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackSlidingAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerSlidingAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackSlidingAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBoomerangAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBoomerangAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBoomerangAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBoomerangAttackCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBoomerangAttackCollide(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBoomerangAttackCollide(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBoomerangReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBoomerangReflect(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBoomerangReflect(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBoomerangBreak message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBoomerangBreak(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBoomerangBreak(), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBodyAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBodyAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBodyAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBodyLanding message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBodyLanding(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBodyLanding(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackBodyAttackReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerBodyAttackReflect(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackBodyAttackReflect(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackClimbAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerClimbAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackClimbAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackSpinAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerSpinAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackSpinAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackGiant message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerGiantAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackGiant(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerCooperationHipDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCooperationHipDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerCooperationHipDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackClimbSliding message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerClimbSlidingAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackClimbSliding(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerAttackClimbRolling message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerClimbRollingAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerAttackClimbRolling(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerGiantHipDrop message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerGiantHipDrop(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerGiantHipDrop(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerGiantTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerGiantTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerGiantTouch(), pReceiver, pSender);
}

/**
 * Sends a PlayerDisregard message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerDisregard(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerDisregard(), pReceiver, pSender);
}

/**
 * Sends a PlayerItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerItemGet(), pReceiver, pSender);
}

/**
 * Sends a KeyOpen message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKeyOpen(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKeyOpen(), pReceiver, pSender);
}

/**
 * Sends a KeyThrow message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKeyThrow(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKeyThrow(), pReceiver, pSender);
}

/**
 * Sends a PlayerReleaseEquipment message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerReleaseEquipment(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerReleaseEquipment(), pReceiver, pSender);
}

/**
 * Sends a PlayerReleaseEquipmentGoal message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param type The type to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerReleaseEquipmentGoal(HitSensor* pReceiver, HitSensor* pSender, u32 type) {
    return sendMsgSensorToSensor(SensorMsgPlayerReleaseEquipmentGoal(type), pReceiver, pSender);
}

/**
 * Sends a PlayerFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a KoopaJrFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKoopaJrFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKoopaJrFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a PlayerDamageTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerDamageTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerDamageTouch(), pReceiver, pSender);
}

/**
 * Sends a PlayerCarryFront message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCarryFront(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCarryFront(), pReceiver, pSender);
}

/**
 * Sends a PlayerCarryFrontNeko message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCarryFrontNeko(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCarryFrontNeko(), pReceiver, pSender);
}

/**
 * Sends a PlayerCarryUp message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCarryUp(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCarryUp(), pReceiver, pSender);
}

/**
 * Sends a PlayerCarryUpTest message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCarryUpTest(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCarryUpTest(), pReceiver, pSender);
}

/**
 * Sends a PlayerCanCarry message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCanCarry(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCanCarry(), pReceiver, pSender);
}

/**
 * Sends a PlayerLeave message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerLeave(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerLeave(), pReceiver, pSender);
}

/**
 * Sends a PlayerRelease message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerRelease(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerRelease(), pReceiver, pSender);
}

/**
 * Sends a PlayerReleaseDamage message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerReleaseDamage(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerReleaseDamage(), pReceiver, pSender);
}

/**
 * Sends a PlayerReleaseDead message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerReleaseDead(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerReleaseDead(), pReceiver, pSender);
}

/**
 * Sends a PlayerToss message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerToss(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerToss(), pReceiver, pSender);
}

/**
 * Sends a PlayerInvincibleTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerInvincibleTouch(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerInvincibleTouch(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a PlayerHideItem message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerHideItem(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerHideItem(), pReceiver, pSender);
}

/**
 * Sends a PlayerShowItem message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerShowItem(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerShowItem(), pReceiver, pSender);
}

/**
 * Sends a PressureDeath message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPressureDeath(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPressureDeath(), pReceiver, pSender);
}

/**
 * Sends a NpcTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNpcTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNpcTouch(), pReceiver, pSender);
}

/**
 * Sends a EnemyAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyAttack(), pReceiver, pSender);
}

/**
 * Sends a EnemyAttackFire message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttackFire(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyAttackFire(), pReceiver, pSender);
}

/**
 * Sends a EnemyAttackBoomerang message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttackBoomerang(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyAttackBoomerang(), pReceiver, pSender);
}

/**
 * Sends a EnemyRouteDokanAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyRouteDokanAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyRouteDokanAttack(), pReceiver, pSender);
}

/**
 * Sends a EnemyRouteDokanFire message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyRouteDokanFire(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyRouteDokanFire(), pReceiver, pSender);
}

/**
 * Sends a Explosion message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgExplosion(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgExplosion(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a ExplosionCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgExplosionCollide(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgExplosionCollide(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a Push message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPush(), pReceiver, pSender);
}

/**
 * Sends a PushStrong message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPushStrong(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPushStrong(), pReceiver, pSender);
}

/**
 * Sends a PushVeryStrong message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPushVeryStrong(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPushVeryStrong(), pReceiver, pSender);
}

/**
 * Sends a Hit message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHit(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHit(), pReceiver, pSender);
}

/**
 * Sends a HitStrong message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHitStrong(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHitStrong(), pReceiver, pSender);
}

/**
 * Sends a HitVeryStrong message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHitVeryStrong(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHitVeryStrong(), pReceiver, pSender);
}

/**
 * Sends a KnockDown message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKnockDown(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKnockDown(), pReceiver, pSender);
}

/**
 * Sends a Push message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgMapPush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPush(), pReceiver, pSender);
}

/**
 * Sends a Vanish message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgVanish(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgVanish(), pReceiver, pSender);
}

/**
 * Sends a ShowModel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgShowModel(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgShowModel(), pReceiver, pSender);
}

/**
 * Sends a HideModel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHideModel(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHideModel(), pReceiver, pSender);
}

/**
 * Sends a EnemyTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyTouch(), pReceiver, pSender);
}

/**
 * Sends a EnemyFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a EnemyUpperPunch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyUpperPunch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyUpperPunch(), pReceiver, pSender);
}

/**
 * Sends a PunpunFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPunpunFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPunpunFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a InvalidateFootPrint message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgInvalidateFootPrint(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgInvalidateFootPrint(), pReceiver, pSender);
}

/**
 * Sends a KickKouraAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgKickKouraAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a KickKouraAttackCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraAttackCollide(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgKickKouraAttackCollide(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a KickKouraGetItem message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraGetItem(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickKouraGetItem(), pReceiver, pSender);
}

/**
 * Sends a KickKouraReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraReflect(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickKouraReflect(), pReceiver, pSender);
}

/**
 * Sends a KickKouraCollideNoReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraCollideNoReflect(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickKouraCollideNoReflect(), pReceiver, pSender);
}

/**
 * Sends a KickKouraBreak message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraBreak(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickKouraBreak(), pReceiver, pSender);
}

/**
 * Sends a KickKouraBlow message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickKouraBlow(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickKouraBlow(), pReceiver, pSender);
}

/**
 * Sends a KouraDestroy message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKouraDestroy(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKouraDestroy(), pReceiver, pSender);
}

/**
 * Sends a KouraThrow message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKouraThrow(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKouraThrow(), pReceiver, pSender);
}

/**
 * Sends a KickStoneAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKickStoneAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKickStoneAttack(), pReceiver, pSender);
}

/**
 * Sends a KillerAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerAttack(), pReceiver, pSender);
}

/**
 * Sends a KillerReflect message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerReflect(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerReflect(), pReceiver, pSender);
}

/**
 * Sends a LiftGeyser message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgLiftGeyser(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgLiftGeyser(), pReceiver, pSender);
}

/**
 * Sends a WarpStart message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgWarpStart(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgWarpStart(), pReceiver, pSender);
}

/**
 * Sends a WarpEnd message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgWarpEnd(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgWarpEnd(), pReceiver, pSender);
}

/**
 * Sends a HoldCancel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHoldCancel(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHoldCancel(), pReceiver, pSender);
}

/**
 * Sends a HoldCancelWarp message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHoldCancelWarp(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHoldCancelWarp(), pReceiver, pSender);
}

/**
 * Sends a HoleIn message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHoleIn(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHoleIn(), pReceiver, pSender);
}

/**
 * Sends a JumpInhibit message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgJumpInhibit(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgJumpInhibit(), pReceiver, pSender);
}

/**
 * Sends a GoalKill message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGoalKill(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGoalKill(), pReceiver, pSender);
}

/**
 * Sends a Goal message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGoal(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGoal(), pReceiver, pSender);
}

/**
 * Sends a BindStart message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindStart(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindStart(), pReceiver, pSender);
}

/**
 * Sends a BindInit message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param type The type to send.
 * @return Whether the message was received.
 */
bool sendMsgBindInit(HitSensor* pReceiver, HitSensor* pSender, u32 type) {
    return sendMsgSensorToSensor(SensorMsgBindInit(type), pReceiver, pSender);
}

/**
 * Sends a BindEnd message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindEnd(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindEnd(), pReceiver, pSender);
}

/**
 * Sends a BindCancel message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindCancel(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindCancel(), pReceiver, pSender);
}

/**
 * Sends a BindCancelForGoal message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindCancelForGoal(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindCancelForGoal(), pReceiver, pSender);
}

/**
 * Sends a BindCancelForWarp message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindCancelForWarp(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindCancelForWarp(), pReceiver, pSender);
}

/**
 * Sends a BindCancelForDemo message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindCancelForDemo(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindCancelForDemo(), pReceiver, pSender);
}

/**
 * Sends a BindDamage message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindDamage(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindDamage(), pReceiver, pSender);
}

/**
 * Sends a BindSteal message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindSteal(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindSteal(), pReceiver, pSender);
}

/**
 * Sends a BindGiant message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBindGiant(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBindGiant(), pReceiver, pSender);
}

/**
 * Sends a BallAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgBallAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBallAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BallRouteDokanAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgBallRouteDokanAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBallRouteDokanAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BallAttackHold message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBallAttackHold(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBallAttackHold(), pReceiver, pSender);
}

/**
 * Sends a BallAttackDRCHold message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBallAttackDRCHold(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBallAttackDRCHold(), pReceiver, pSender);
}

/**
 * Sends a BallAttackCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBallAttackCollide(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBallAttackCollide(), pReceiver, pSender);
}

/**
 * Sends a BallTrample message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgBallTrample(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBallTrample(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BallTrampleCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBallTrampleCollide(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBallTrampleCollide(), pReceiver, pSender);
}

/**
 * Sends a BallItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBallItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBallItemGet(), pReceiver, pSender);
}

/**
 * Sends a FireBallCollide message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgFireBalCollide(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgFireBallCollide(), pReceiver, pSender);
}

/**
 * Sends a FireBallFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgFireBallFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgFireBallFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a DokanBazookaAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgDokanBazookaAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgDokanBazookaAttack(), pReceiver, pSender);
}

/**
 * Sends a RideAllPlayerItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRideAllPlayerItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRideAllPlayerItemGet(), pReceiver, pSender);
}

/**
 * Sends a PlayerFloorTouch message to the ground sensor the actor collided with.
 * @param pActor The actor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerFloorTouchToColliderGround(LiveActor* pActor, HitSensor* pSender) {
    return sendMsgPlayerFloorTouch(tryGetCollidedGroundSensor(pActor), pSender);
}

/**
 * Sends a PlayerUpperPunch message to the ceiling sensor the actor collided with.
 * @param pActor The actor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerUpperPunchToColliderCeiling(LiveActor* pActor, HitSensor* pSender) {
    return sendMsgPlayerUpperPunch(tryGetCollidedCeilingSensor(pActor), pSender);
}

/**
 * Sends an EnemyFloorTouch message to the ground sensor the actor collided with.
 * @param pActor The actor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyFloorTouchToColliderGround(LiveActor* pActor, HitSensor* pSender) {
    return sendMsgEnemyFloorTouch(tryGetCollidedGroundSensor(pActor), pSender);
}

/**
 * Sends an EnemyUpperPunch message to the ceiling sensor the actor collided with.
 * @param pActor The actor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyUpperPunchToColliderCeiling(LiveActor* pActor, HitSensor* pSender) {
    return sendMsgEnemyUpperPunch(tryGetCollidedCeilingSensor(pActor), pSender);
}

/**
 * Sends a AskSafetyPoint message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgAskSafetyPoint(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgAskSafetyPoint(), pReceiver, pSender);
}

/**
 * Sends a TouchAssist message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgTouchAssist(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgTouchAssist(), pReceiver, pSender);
}

/**
 * Sends a TouchAssistTrig message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgTouchAssistTrig(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgTouchAssistTrig(), pReceiver, pSender);
}

/**
 * Sends a StrokeTransparent message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgStrokeTransparent(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgStrokeTransparent(), pReceiver, pSender);
}

/**
 * Sends a ScreenPointInvalidCollisionParts message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgScreenPointInvalidCollisionParts(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgScreenPointInvalidCollisionParts(), pReceiver, pSender);
}

/**
 * Sends a BlockUpperPunch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgBlockUpperPunch(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBlockUpperPunch(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BlockLowerPunch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgBlockLowerPunch(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgBlockLowerPunch(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a BlockItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBlockItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBlockItemGet(), pReceiver, pSender);
}

/**
 * Sends a KillerItemGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKillerItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKillerItemGet(), pReceiver, pSender);
}

/**
 * Sends a PlayerKouraAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param pComboCounter The combo counter to send.
 * @return Whether the message was received.
 */
bool sendMsgPlayerKouraAttack(HitSensor* pReceiver, HitSensor* pSender, ComboCounter* pComboCounter) {
    return sendMsgSensorToSensor(SensorMsgPlayerKouraAttack(pComboCounter), pReceiver, pSender);
}

/**
 * Sends a LightFlash message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgLightFlash(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgLightFlash(), pReceiver, pSender);
}

/**
 * Sends a HeadlightFlash message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgHeadlightFlash(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgHeadlightFlash(), pReceiver, pSender);
}

/**
 * Sends a ForceAbyss message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgForceAbyss(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgForceAbyss(), pReceiver, pSender);
}

/**
 * Sends a IsNerveSupportFreeze message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsNerveSupportFreeze(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsNerveSupportFreeze(), pReceiver, pSender);
}

/**
 * Sends a OnSyncSupportFreeze message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgOnSyncSupportFreeze(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgOnSyncSupportFreeze(), pReceiver, pSender);
}

/**
 * Sends a OffSyncSupportFreeze message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgOffSyncSupportFreeze(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgOffSyncSupportFreeze(), pReceiver, pSender);
}

/**
 * Sends an AskSafetyPoint message to the ground sensor the actor collided with.
 * @param pActor The actor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgAskSafetyPointToColliderGround(LiveActor* pActor, HitSensor* pSender) {
    return sendMsgAskSafetyPoint(tryGetCollidedGroundSensor(pActor), pSender);
}

/**
 * Sends a SwordAttackHighLeft message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordAttackHighLeft(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordAttackHighLeft(), pReceiver, pSender);
}

/**
 * Sends a SwordAttackLowLeft message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordAttackLowLeft(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordAttackLowLeft(), pReceiver, pSender);
}

/**
 * Sends a SwordAttackHighRight message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordAttackHighRight(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordAttackHighRight(), pReceiver, pSender);
}

/**
 * Sends a SwordAttackLowRight message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordAttackLowRight(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordAttackLowRight(), pReceiver, pSender);
}

/**
 * Sends a SwordAttackJumpUnder message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordAttackJumpUnder(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordAttackJumpUnder(), pReceiver, pSender);
}

/**
 * Sends a SwordBeamAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordBeamAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordBeamAttack(), pReceiver, pSender);
}

/**
 * Sends a SwordBeamReflectAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSwordBeamReflectAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSwordBeamReflectAttack(), pReceiver, pSender);
}

/**
 * Sends a ShieldGuard message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgShieldGuard(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgShieldGuard(), pReceiver, pSender);
}

/**
 * Sends a EnemyAttackKnockDown message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttackKnockDown(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgEnemyAttackKnockDown(), pReceiver, pSender);
}

/**
 * Sends a AskMultiPlayerEnemy message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgAskMultiPlayerEnemy(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgAskMultiPlayerEnemy(), pReceiver, pSender);
}

/**
 * Sends a ItemGettable message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgItemGettable(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgItemGettable(), pReceiver, pSender);
}

/**
 * Sends a KikkiThrow message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgKikkiThrow(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgKikkiThrow(), pReceiver, pSender);
}

/**
 * Sends a IsKikkiThrowTarget message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsKikkiThrowTarget(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsKikkiThrowTarget(), pReceiver, pSender);
}

/**
 * Sends a PlayerCloudGet message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlayerCloudGet(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlayerCloudGet(), pReceiver, pSender);
}

/**
 * Sends a AutoJump message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgAutoJump(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgAutoJump(), pReceiver, pSender);
}

/**
 * Sends a Sink message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgSink(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgSink(), pReceiver, pSender);
}

/**
 * Sends a LaserAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgLaserAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgLaserAttack(), pReceiver, pSender);
}

/**
 * Sends a GigaStomp message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGigaStomp(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGigaStomp(), pReceiver, pSender);
}

/**
 * Sends a Restore message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgRestore(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgRestore(), pReceiver, pSender);
}

/**
 * Sends a GigaEnemyAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGigaEnemyAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGigaEnemyAttack(), pReceiver, pSender);
}

/**
 * Sends a NekoAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNekoAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNekoAttack(), pReceiver, pSender);
}

/**
 * Sends a NekoPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgNekoPush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgNekoPush(), pReceiver, pSender);
}

/**
 * Sends a DisasterSpikeAttack message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgDisasterSpikeAttack(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgDisasterSpikeAttack(), pReceiver, pSender);
}

/**
 * Sends a DisasterSpikePush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgDisasterSpikePush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgDisasterSpikePush(), pReceiver, pSender);
}

/**
 * Sends a PlessieFloorTouch message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgPlessieFloorTouch(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgPlessieFloorTouch(), pReceiver, pSender);
}

/**
 * Sends a GigaBellPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgGigaBellPush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgGigaBellPush(), pReceiver, pSender);
}

/**
 * Sends a BowserPush message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgBowserPush(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgBowserPush(), pReceiver, pSender);
}

/**
 * Sends a CutsceneStart message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgCutsceneStart(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgCutsceneStart(), pReceiver, pSender);
}

/**
 * Sends a IsItemHomingTarget message from one sensor to another.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgIsItemHomingTarget(HitSensor* pReceiver, HitSensor* pSender) {
    return sendMsgSensorToSensor(SensorMsgIsItemHomingTarget(), pReceiver, pSender);
}

/**
 * Checks whether a message is EnemyAttack or EnemyAttackFire or EnemyAttackKnockDown.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyAttack>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgEnemyAttackFire>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgEnemyAttackKnockDown>(pMsg);
}

/**
 * Checks whether a message is EnemyAttackFire.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyAttackFire(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyAttackFire>(pMsg);
}

/**
 * Checks whether a message is EnemyAttackKnockDown.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyAttackKnockDown(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyAttackKnockDown>(pMsg);
}

/**
 * Checks whether a message is EnemyAttackBoomerang.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyAttackBoomerang(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyAttackBoomerang>(pMsg);
}

/**
 * Checks whether a message is EnemyRouteDokanAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyRouteDokanAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyRouteDokanAttack>(pMsg);
}

/**
 * Checks whether a message is EnemyRouteDokanFire.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyRouteDokanFire(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyRouteDokanFire>(pMsg);
}

/**
 * Checks whether a message is Explosion.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgExplosion(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgExplosion>(pMsg);
}

/**
 * Checks whether a message is ExplosionCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgExplosionCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgExplosionCollide>(pMsg);
}

/**
 * Checks whether a message is BindStart.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindStart(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindStart>(pMsg);
}

/**
 * Checks whether a message is BindInit.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindInit(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindInit>(pMsg);
}

/**
 * Checks whether a message is BindEnd.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindEnd(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindEnd>(pMsg);
}

/**
 * Checks whether a message is BindCancel or BindCancelForGoal or BindCancelForDemo or BindCancelForWarp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindCancel(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindCancel>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgBindCancelForGoal>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgBindCancelForDemo>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgBindCancelForWarp>(pMsg);
}

/**
 * Checks whether a message is BindCancelForGoal.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindCancelForGoal(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindCancelForGoal>(pMsg);
}

/**
 * Checks whether a message is BindCancelForWarp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindCancelForWarp(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindCancelForWarp>(pMsg);
}

/**
 * Checks whether a message is BindCancelForDemo.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindCancelForDemo(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindCancelForDemo>(pMsg);
}

/**
 * Checks whether a message is BindDamage.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindDamage(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindDamage>(pMsg);
}

/**
 * Checks whether a message is BindSteal.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindSteal(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindSteal>(pMsg);
}

/**
 * Checks whether a message is BindGiant.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBindGiant(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBindGiant>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackTrample.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerTrample(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackTrample>(pMsg);
}

/**
 * Checks whether a message is KeyOpen.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKeyOpen(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKeyOpen>(pMsg);
}

/**
 * Checks whether a message is KeyThrow.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKeyThrow(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKeyThrow>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackTrample or BallTrample.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTrampleAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackTrample>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgBallTrample>(pMsg);
}

/**
 * Checks whether a message is BallTrample.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallTrample(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallTrample>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackHipDrop or PlayerAttackStatueDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerHipDropAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackHipDrop>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlayerAttackStatueDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackStatueDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerStatueDrop(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackStatueDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjHipDrop or PlayerAttackObjStatueDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjHipDropAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjHipDrop>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlayerAttackObjStatueDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjStatueDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjStatueDrop(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjStatueDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjHipDropReflect or PlayerAttackObjStatueDropReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjHipDropReflectAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjHipDropReflect>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlayerAttackObjStatueDropReflect>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjStatueDropReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjStatueDropReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjStatueDropReflect>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjHipDropHighJump.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjHipDropHighJump(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjHipDropHighJump>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackHipDropKnockDown.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerHipDropKnockDown(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackHipDropKnockDown>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjStatueDropReflectNoCondition.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjStatueDropReflectNoCondition(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjStatueDropReflectNoCondition>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackStatueTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerStatueTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackStatueTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackUpperPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerUpperPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackUpperPunch>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjUpperPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjUpperPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjUpperPunch>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackRollingAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerRollingAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackRollingAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackRollingReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerRollingReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackRollingReflect>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjRollingAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjRollingAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjRollingAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackObjRollingAttackFailure.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjRollingAttackFailure(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackObjRollingAttackFailure>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackInvincibleAttack or PlayerAttackInvincibleHelpAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerInvincibleAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackInvincibleAttack>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlayerAttackInvincibleHelpAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackInvincibleHelpAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerInvincibleHelpAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackInvincibleHelpAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackInvincibleAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerOnlyInvincibleAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackInvincibleAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackFireBallAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerFireBallAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackFireBallAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackRouteDokanFireBallAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerRouteDokanFireBallAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackRouteDokanFireBallAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackTailAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerTailAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackTailAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackKick.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerKick(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackKick>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackCatch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCatch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackCatch>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackSlidingAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerSlidingAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackSlidingAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBoomerangAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBoomerangAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBoomerangAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBoomerangAttackCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBoomerangAttackCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBoomerangAttackCollide>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBoomerangReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBoomerangReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBoomerangReflect>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBoomerangBreak.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBoomerangBreak(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBoomerangBreak>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBodyAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBodyAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBodyAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBodyLanding.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBodyLanding(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBodyLanding>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackBodyAttackReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerBodyAttackReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackBodyAttackReflect>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackClimbAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerClimbAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackClimbAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackSpinAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerSpinAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackSpinAttack>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackGiant.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerGiantAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackGiant>(pMsg);
}

/**
 * Checks whether a message is PlayerCooperationHipDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCooperationHipDrop(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCooperationHipDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackClimbSliding.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerClimbSlidingAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackClimbSliding>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackClimbRolling.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerClimbRollingAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackClimbRolling>(pMsg);
}

/**
 * Checks whether a message is PlayerGiantHipDrop.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerGiantHipDrop(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerGiantHipDrop>(pMsg);
}

/**
 * Checks whether a message is PlayerDisregard.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerDisregard(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerDisregard>(pMsg);
}

/**
 * Checks whether a message is PlayerItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerItemGet>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackDash.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerDash(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackDash>(pMsg);
}

/**
 * Checks whether a message is PlayerFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerFloorTouch>(pMsg);
}

/**
 * Checks whether a message is KoopaJrFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKoopaJrFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKoopaJrFloorTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerDamageTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerDamageTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerDamageTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerFloorTouchBind.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerFloorTouchBind(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerFloorTouchBind>(pMsg);
}

/**
 * Checks whether a message is PlayerTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerInvincibleTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerInvincibleTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerInvincibleTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerGiantTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerGiantTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerGiantTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerObjTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerItemGet>(pMsg);
}

/**
 * Checks whether a message is PlayerReleaseEquipment.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerReleaseEquipment(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerReleaseEquipment>(pMsg);
}

/**
 * Checks whether a message is PlayerReleaseEquipmentGoal.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerReleaseEquipmentGoal(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerReleaseEquipmentGoal>(pMsg);
}

/**
 * Checks whether a message is PlayerCarryFront.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCarryFront(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCarryFront>(pMsg);
}

/**
 * Checks whether a message is PlayerCarryFrontNeko.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCarryFrontNeko(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCarryFrontNeko>(pMsg);
}

/**
 * Checks whether a message is PlayerCarryUp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCarryUp(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCarryUp>(pMsg);
}

/**
 * Checks whether a message is PlayerCarryUpTest.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCarryUpTest(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCarryUpTest>(pMsg);
}

/**
 * Checks whether a message is PlayerCanCarry.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCanCarry(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCanCarry>(pMsg);
}

/**
 * Checks whether a message is PlayerLeave.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerLeave(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerLeave>(pMsg);
}

/**
 * Checks whether a message is PlayerRelease.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerRelease(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerRelease>(pMsg);
}

/**
 * Checks whether a message is PlayerReleaseDamage.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerReleaseDamage(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerReleaseDamage>(pMsg);
}

/**
 * Checks whether a message is PlayerReleaseDead.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerReleaseDead(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerReleaseDead>(pMsg);
}

/**
 * Checks whether a message is PlayerToss.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerToss(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerToss>(pMsg);
}

/**
 * Checks whether a message is PlayerHideItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerHideItem(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerHideItem>(pMsg);
}

/**
 * Checks whether a message is PlayerShowItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerShowItem(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerShowItem>(pMsg);
}

/**
 * Checks whether a message is PressureDeath.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPressureDeath(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPressureDeath>(pMsg);
}

/**
 * Checks whether a message is NpcTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNpcTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNpcTouch>(pMsg);
}

/**
 * Checks whether a message is Push.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPush>(pMsg);
}

/**
 * Checks whether a message is PushStrong.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPushStrong(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPushStrong>(pMsg);
}

/**
 * Checks whether a message is PushVeryStrong.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPushVeryStrong(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPushVeryStrong>(pMsg);
}

/**
 * Checks whether a message is Hit.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHit(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHit>(pMsg);
}

/**
 * Checks whether a message is HitStrong.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHitStrong(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHitStrong>(pMsg);
}

/**
 * Checks whether a message is HitVeryStrong.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHitVeryStrong(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHitVeryStrong>(pMsg);
}

/**
 * Checks whether a message is KnockDown.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKnockDown(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKnockDown>(pMsg);
}

/**
 * Checks whether a message is Push.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgMapPush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPush>(pMsg);
}

/**
 * Checks whether a message is Vanish.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgVanish(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgVanish>(pMsg);
}

/**
 * Checks whether a message is ShowModel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgShowModel(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgShowModel>(pMsg);
}

/**
 * Checks whether a message is HideModel.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHideModel(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHideModel>(pMsg);
}

/**
 * Checks whether a message is EnemyTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyTouch>(pMsg);
}

/**
 * Checks whether a message is EnemyFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyFloorTouch>(pMsg);
}

/**
 * Checks whether a message is EnemyUpperPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgEnemyUpperPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgEnemyUpperPunch>(pMsg);
}

/**
 * Checks whether a message is PunpunFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPunpunFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPunpunFloorTouch>(pMsg);
}

/**
 * Checks whether a message is InvalidateFootPrint.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgInvalidateFootPrint(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgInvalidateFootPrint>(pMsg);
}

/**
 * Checks whether a message is KickKouraAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraAttack>(pMsg);
}

/**
 * Checks whether a message is KickKouraAttackCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraAttackCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraAttackCollide>(pMsg);
}

/**
 * Checks whether a message is KickKouraReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraReflect>(pMsg);
}

/**
 * Checks whether a message is KickKouraCollideNoReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraCollideNoReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraCollideNoReflect>(pMsg);
}

/**
 * Checks whether a message is KickKouraBreak.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraBreak(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraBreak>(pMsg);
}

/**
 * Checks whether a message is KickKouraBlow.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraBlow(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraBlow>(pMsg);
}

/**
 * Checks whether a message is KouraDestroy.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKouraDestroy(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKouraDestroy>(pMsg);
}

/**
 * Checks whether a message is KickKouraGetItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickKouraItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickKouraGetItem>(pMsg);
}

/**
 * Checks whether a message is KickStoneAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKickStoneAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKickStoneAttack>(pMsg);
}

/**
 * Checks whether a message is KillerAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerAttack>(pMsg);
}

/**
 * Checks whether a message is KillerReflect.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerReflect(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerReflect>(pMsg);
}

/**
 * Checks whether a message is KouraThrow.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKouraThrow(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKouraThrow>(pMsg);
}

/**
 * Checks whether a message is LiftGeyser.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgLiftGeyser(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgLiftGeyser>(pMsg);
}

/**
 * Checks whether a message is WarpStart.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgWarpStart(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgWarpStart>(pMsg);
}

/**
 * Checks whether a message is WarpEnd.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgWarpEnd(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgWarpEnd>(pMsg);
}

/**
 * Checks whether a message is HoldCancel or HoldCancelWarp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHoldCancel(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHoldCancel>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgHoldCancelWarp>(pMsg);
}

/**
 * Checks whether a message is HoldCancelWarp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHoldCancelWarp(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHoldCancelWarp>(pMsg);
}

/**
 * Checks whether a message is HoleIn.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHoleIn(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHoleIn>(pMsg);
}

/**
 * Checks whether a message is JumpInhibit.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgJumpInhibit(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgJumpInhibit>(pMsg);
}

/**
 * Checks whether a message is GoalKill.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGoalKill(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGoalKill>(pMsg);
}

/**
 * Checks whether a message is Goal.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGoal(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGoal>(pMsg);
}

/**
 * Checks whether a message is BallAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallAttack>(pMsg);
}

/**
 * Checks whether a message is BallRouteDokanAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallRouteDokanAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallRouteDokanAttack>(pMsg);
}

/**
 * Checks whether a message is BallAttackHold.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallAttackHold(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallAttackHold>(pMsg);
}

/**
 * Checks whether a message is BallAttackDRCHold.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallAttackDRCHold(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallAttackDRCHold>(pMsg);
}

/**
 * Checks whether a message is BallAttackCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallAttackCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallAttackCollide>(pMsg);
}

/**
 * Checks whether a message is BallTrampleCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallTrampleCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallTrampleCollide>(pMsg);
}

/**
 * Checks whether a message is BallItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBallItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallItemGet>(pMsg);
}

/**
 * Checks whether a message is FireBallCollide.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgFireBallCollide(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgFireBallCollide>(pMsg);
}

/**
 * Checks whether a message is FireBallFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgFireBallFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgFireBallFloorTouch>(pMsg);
}

/**
 * Checks whether a message is DokanBazookaAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDokanBazookaAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDokanBazookaAttack>(pMsg);
}

/**
 * Checks whether a message is RideAllPlayerItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRideAllPlayerItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRideAllPlayerItemGet>(pMsg);
}

/**
 * Checks whether a message is PlayerItemGet or RideAllPlayerItemGet or PlayerAttackTailAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemGetDirectAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerItemGet>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgRideAllPlayerItemGet>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlayerAttackTailAttack>(pMsg);
}

/**
 * Checks whether a message is BallItemGet or KickKouraGetItem or KillerItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemGetByObjAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBallItemGet>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgKickKouraGetItem>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgKillerItemGet>(pMsg);
}

/**
 * Checks whether a message is KillerItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKillerItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKillerItemGet>(pMsg);
}

/**
 * Checks whether a message is any item get message.
 * @param pMsg The message.
 * @return Whether the message is an item get message.
 */
bool isMsgItemGetAll(const SensorMsg* pMsg) {
    return isMsgItemGetDirectAll(pMsg) || isMsgItemGetByObjAll(pMsg);
}

/**
 * Checks whether a message is PlayerFloorTouch or EnemyFloorTouch or PlessieFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerFloorTouch>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgEnemyFloorTouch>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgPlessieFloorTouch>(pMsg);
}

/**
 * Checks whether a message is PlessieFloorTouch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlessieFloorTouch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlessieFloorTouch>(pMsg);
}

/**
 * Checks whether a message is PlayerAttackUpperPunch or EnemyUpperPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgUpperPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerAttackUpperPunch>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgEnemyUpperPunch>(pMsg);
}

/**
 * Checks whether a message is AskSafetyPoint.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgAskSafetyPoint(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgAskSafetyPoint>(pMsg);
}

/**
 * Checks whether a message is TouchAssist.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssist(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssist>(pMsg);
}

/**
 * Checks whether a message is TouchAssistNoPat.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistNoPat(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssistNoPat>(pMsg);
}

/**
 * Checks whether a message is TouchAssistTrig.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistTrig(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssistTrig>(pMsg);
}

/**
 * Checks whether a message is TouchAssistTrigNoPat.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistTrigNoPat(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssistTrigNoPat>(pMsg);
}

/**
 * Checks whether a message is TouchAssistBurn.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistBurn(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssistBurn>(pMsg);
}

/**
 * Checks whether a message is TouchAssist or TouchAssistTrig.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchAssistAll(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchAssist>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgTouchAssistTrig>(pMsg);
}

/**
 * Checks whether a message is TouchCarryItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchCarryItem(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchCarryItem>(pMsg);
}

/**
 * Checks whether a message is TouchReleaseItem.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgTouchReleaseItem(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgTouchReleaseItem>(pMsg);
}

/**
 * Checks whether a message is IsNerveSupportFreeze.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsNerveSupportFreeze(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsNerveSupportFreeze>(pMsg);
}

/**
 * Checks whether a message is OnSyncSupportFreeze.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgOnSyncSupportFreeze(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgOnSyncSupportFreeze>(pMsg);
}

/**
 * Checks whether a message is OffSyncSupportFreeze.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgOffSyncSupportFreeze(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgOffSyncSupportFreeze>(pMsg);
}

/**
 * Checks whether a message is StrokeTransparent.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgStrokeTransparent(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgStrokeTransparent>(pMsg);
}

/**
 * Checks whether a message is ScreenPointInvalidCollisionParts.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgScreenPointInvalidCollisionParts(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgScreenPointInvalidCollisionParts>(pMsg);
}

/**
 * Checks whether a message is BlockUpperPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBlockUpperPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBlockUpperPunch>(pMsg);
}

/**
 * Checks whether a message is BlockLowerPunch.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBlockLowerPunch(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBlockLowerPunch>(pMsg);
}

/**
 * Checks whether a message is BlockItemGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBlockItemGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBlockItemGet>(pMsg);
}

/**
 * Checks whether a message is PlayerKouraAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerKouraAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerKouraAttack>(pMsg);
}

/**
 * Checks whether a message is LightFlash.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgLightFlash(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgLightFlash>(pMsg);
}

/**
 * Checks whether a message is HeadlightFlash.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgHeadlightFlash(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgHeadlightFlash>(pMsg);
}

/**
 * Checks whether a message is ForceAbyss.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgForceAbyss(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgForceAbyss>(pMsg);
}

/**
 * Checks whether a message is SwordAttackHighLeft or SwordAttackHighRight.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackHigh(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackHighLeft>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgSwordAttackHighRight>(pMsg);
}

/**
 * Checks whether a message is SwordAttackHighLeft.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackHighLeft(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackHighLeft>(pMsg);
}

/**
 * Checks whether a message is SwordAttackHighRight.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackHighRight(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackHighRight>(pMsg);
}

/**
 * Checks whether a message is SwordAttackLowLeft or SwordAttackLowRight.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackLow(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackLowLeft>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgSwordAttackLowRight>(pMsg);
}

/**
 * Checks whether a message is SwordAttackLowLeft.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackLowLeft(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackLowLeft>(pMsg);
}

/**
 * Checks whether a message is SwordAttackLowRight.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackLowRight(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackLowRight>(pMsg);
}

/**
 * Checks whether a message is SwordBeamAttack or SwordBeamReflectAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordBeamAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordBeamAttack>(pMsg) ||
           sead::IsDerivedFrom<SensorMsgSwordBeamReflectAttack>(pMsg);
}

/**
 * Checks whether a message is SwordBeamReflectAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordBeamReflectAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordBeamReflectAttack>(pMsg);
}

/**
 * Checks whether a message is SwordAttackJumpUnder.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSwordAttackJumpUnder(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSwordAttackJumpUnder>(pMsg);
}

/**
 * Checks whether a message is ShieldGuard.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgShieldGuard(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgShieldGuard>(pMsg);
}

/**
 * Checks whether a message is AskMultiPlayerEnemy.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgAskMultiPlayerEnemy(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgAskMultiPlayerEnemy>(pMsg);
}

/**
 * Checks whether a message is ItemGettable.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgItemGettable(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgItemGettable>(pMsg);
}

/**
 * Checks whether a message is KikkiThrow.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgKikkiThrow(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgKikkiThrow>(pMsg);
}

/**
 * Checks whether a message is IsKikkiThrowTarget.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsKikkiThrowTarget(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsKikkiThrowTarget>(pMsg);
}

/**
 * Checks whether a message is PlayerCloudGet.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgPlayerCloudGet(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgPlayerCloudGet>(pMsg);
}

/**
 * Checks whether a message is AutoJump.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgAutoJump(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgAutoJump>(pMsg);
}

/**
 * Checks whether a message is Sink.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgSink(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgSink>(pMsg);
}

/**
 * Checks whether a message is LaserAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgLaserAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgLaserAttack>(pMsg);
}

/**
 * Checks whether a message is GigaStomp.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGigaStomp(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGigaStomp>(pMsg);
}

/**
 * Checks whether a message is Restore.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgRestore(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgRestore>(pMsg);
}

/**
 * Checks whether a message is GigaEnemyAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGigaEnemyAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGigaEnemyAttack>(pMsg);
}

/**
 * Checks whether a message is NekoAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNekoAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNekoAttack>(pMsg);
}

/**
 * Checks whether a message is NekoPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgNekoPush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgNekoPush>(pMsg);
}

/**
 * Checks whether a message is DisasterSpikeAttack.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDisasterSpikeAttack(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDisasterSpikeAttack>(pMsg);
}

/**
 * Checks whether a message is DisasterSpikePush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgDisasterSpikePush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgDisasterSpikePush>(pMsg);
}

/**
 * Checks whether a message is GigaBellPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgGigaBellPush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgGigaBellPush>(pMsg);
}

/**
 * Checks whether a message is BowserPush.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgBowserPush(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgBowserPush>(pMsg);
}

/**
 * Checks whether a message is CutsceneStart.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgCutsceneStart(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgCutsceneStart>(pMsg);
}

/**
 * Checks whether a message is IsItemHomingTarget.
 * @param pMsg The message.
 * @return Whether the message has that type.
 */
bool isMsgIsItemHomingTarget(const SensorMsg* pMsg) {
    return sead::IsDerivedFrom<SensorMsgIsItemHomingTarget>(pMsg);
}

/**
 * Checks whether a message is a trample coming from above the receiving sensor.
 * @param pMsg The message.
 * @param pSelf The receiving sensor.
 * @param pOther The sending sensor.
 * @return Whether the message is a valid trample.
 */
bool isMsgPlayerTrampleForCrossoverSensor(const SensorMsg* pMsg, const HitSensor* pSelf,
                                          const HitSensor* pOther) {
    if (!isMsgPlayerTrample(pMsg)) {
        return false;
    }

    return isCrossoverSensor(pSelf, pOther);
}

static bool isCrossoverSensor(const HitSensor* pSelf, const HitSensor* pOther) {
    sead::Vector3f dir = pSelf->mPos - pOther->mPos;
    sead::Vector3f gravity = getGravity(pOther->mHostActor);
    if (normalizeOrZero(&dir)) {
        return false;
    }

    sead::Vector3f up = -gravity;
    f32 dot = dir.dot(up);
    if (dot < 0.34202015f) {
        return false;
    }

    if (dot < 0.9659258f && pSelf->mSensorType == HitSensorType::KoopaJr) {
        return false;
    }

    sead::Vector3f velDir;
    normalizeOrZero(&velDir, getVelocity(pSelf->mHostActor));
    if (dir.y < 0.0f) {
        return false;
    }

    f32 velDot = velDir.dot(up);
    if (isNearZero(sead::Mathf::abs(velDot), 0.001f)) {
        return false;
    }

    return !isNearZero(velDot - 1.0f, 0.001f);
}

/**
 * Checks whether a message is an upper punch coming from below the receiving sensor.
 * @param pMsg The message.
 * @param pSelf The receiving sensor.
 * @param pOther The sending sensor.
 * @param speed The minimum upwards speed of the receiver.
 * @return Whether the message is a valid upper punch.
 */
bool isMsgPlayerUpperPunchForCrossoverSensor(const SensorMsg* pMsg, const HitSensor* pSelf,
                                             const HitSensor* pOther, f32 speed) {
    if (!isMsgPlayerObjUpperPunch(pMsg)) {
        return false;
    }

    sead::Vector3f dir = pSelf->mPos - pOther->mPos;
    sead::Vector3f gravity = getGravity(pOther->mHostActor);
    normalize(&dir);
    if (gravity.dot(dir) < 0.34202015f) {
        return false;
    }

    return !(gravity.dot(getVelocity(pSelf->mHostActor)) >= -speed);
}

/**
 * Sends an EnemyAttack message unless the sender is crossing the receiver from above.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttackForCrossoverSensor(HitSensor* pReceiver, HitSensor* pSender) {
    if (isCrossoverSensor(pReceiver, pSender)) {
        return false;
    }

    return sendMsgEnemyAttack(pReceiver, pSender);
}

/**
 * Sends an EnemyAttack message for a cylinder unless the sender is crossing it from above.
 * @param pReceiver The receiving sensor.
 * @param pSender The sending sensor.
 * @param rPos A point on the cylinder axis.
 * @param rAxis The cylinder axis.
 * @param radius The cylinder radius.
 * @return Whether the message was received.
 */
bool sendMsgEnemyAttackForCrossoverCylinderSensor(HitSensor* pReceiver, HitSensor* pSender,
                                                  const sead::Vector3f& rPos,
                                                  const sead::Vector3f& rAxis, f32 radius) {
    f32 innerRadius = sead::Mathf::clamp(radius - 20.0f, 0.0f, radius);
    {
        sead::Vector3f dir = pReceiver->mPos - pSender->mPos;
        sead::Vector3f gravity = getGravity(pSender->mHostActor);
        verticalizeVec(&dir, rAxis, dir);
        dir.length();
        if (!(dir.squaredLength() < innerRadius * innerRadius)) {
            normalize(&dir);
            sead::Vector3f velDir;
            normalizeOrZero(&velDir, getVelocity(pSender->mHostActor));
            if (!(dir.y < 0.0f)) {
                f32 velDot = velDir.dot(-gravity);
                if (!isNearZero(sead::Mathf::abs(velDot), 0.001f) &&
                    !isNearZero(velDot - 1.0f, 0.001f)) {
                    return false;
                }
            }
        }
    }

    if (!isHitCylinderSensor(pReceiver, rPos, rAxis, radius)) {
        return false;
    }

    return sendMsgEnemyAttack(pReceiver, pSender);
}

/**
 * Checks the type of a sensor.
 * @param pSensor The sensor.
 * @param type The type to compare with.
 * @return Whether the sensor has that type.
 */
bool isSensorType(const HitSensor* pSensor, s32 type) {
    return pSensor->mSensorType == static_cast<HitSensorType>(type);
}

/**
 * Checks whether a sensor is a player or player eye sensor.
 * @param pSensor The sensor.
 * @return Whether the sensor belongs to a player.
 */
bool isSensorPlayer(const HitSensor* pSensor) {
    return isSensorPlayerType(pSensor) || isSensorPlayerEye(pSensor);
}

/**
 * Checks whether a sensor has the Player type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorPlayerType(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Player;
}

/**
 * Checks whether a sensor has the PlayerEye type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorPlayerEye(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::PlayerEye;
}

/**
 * Checks whether a sensor belongs to a player or a player's fire ball.
 * @param pSensor The sensor.
 * @return Whether the sensor belongs to a player or its weapon.
 */
bool isSensorPlayerOrPlayerWeapon(const HitSensor* pSensor) {
    return isSensorPlayer(pSensor) || isSensorPlayerFireBall(pSensor);
}

/**
 * Checks whether a sensor has the KickKoura type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorKickKoura(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::KickKoura;
}

/**
 * Checks whether a sensor has the Npc type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorNpc(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Npc;
}

/**
 * Checks whether a sensor has the NpcAvoid type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorNpcAvoid(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::NpcAvoid;
}

/**
 * Checks whether a sensor has the Ride type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorRide(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Ride;
}

/**
 * Checks whether a sensor has the Eye type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorEye(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Eye;
}

/**
 * Checks whether a sensor is any enemy sensor.
 * @param pSensor The sensor.
 * @return Whether the sensor is an enemy sensor.
 */
bool isSensorEnemy(const HitSensor* pSensor) {
    return isSensorEnemyType(pSensor) || isSensorEnemyBody(pSensor) ||
           isSensorEnemyAttack(pSensor) || isSensorKillerMagnum(pSensor) ||
           isSensorDossun(pSensor);
}

/**
 * Checks whether a sensor has the Enemy type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorEnemyType(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Enemy;
}

/**
 * Checks whether a sensor has the EnemyBody type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorEnemyBody(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::EnemyBody;
}

/**
 * Checks whether a sensor has the EnemyAttack type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorEnemyAttack(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::EnemyAttack;
}

/**
 * Checks whether a sensor has the Dossun type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorDossun(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Dossun;
}

/**
 * Checks whether a sensor has the KillerMagnum type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorKillerMagnum(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::KillerMagnum;
}

/**
 * Checks whether a sensor has the MapObj type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorMapObj(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::MapObj;
}

/**
 * Checks whether a sensor has the CollisionParts type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorCollision(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::CollisionParts;
}

/**
 * Checks whether a sensor has the PlayerFireBall type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorPlayerFireBall(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::PlayerFireBall;
}

/**
 * Checks whether a sensor has the HoldObj type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorHoldObj(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::HoldObj;
}

/**
 * Checks whether a sensor has the MultiPlayer type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorMultiPlayer(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::MultiPlayer;
}

/**
 * Checks whether a sensor has the KoopaJr type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorKoopaJr(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::KoopaJr;
}

/**
 * Checks whether a sensor has the BindableGigaBell type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableGigaBell(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableGigaBell;
}

/**
 * Checks whether a sensor has the BindableGoal type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableGoal(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableGoal;
}

/**
 * Checks whether a sensor has the BindableGoalItem type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableGoalItem(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableGoalItem;
}

/**
 * Checks whether a sensor has the BindableAllPlayer type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableAllPlayer(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableAllPlayer;
}

/**
 * Checks whether a sensor has the BindableBubbleOutScreen type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableBubbleOutScreen(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableBubbleOutScreen;
}

/**
 * Checks whether a sensor has the BindableKoura type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableKoura(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableKoura;
}

/**
 * Checks whether a sensor has the BindableNpc type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableNpc(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableNpc;
}

/**
 * Checks whether a sensor has the BindableRouteDokan type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableRouteDokan(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableRouteDokan;
}

/**
 * Checks whether a sensor has the BindableBubblePadInput type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindableBubblePadInput(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::BindableBubblePadInput;
}

/**
 * Checks whether a sensor has the Bindable type.
 * @param pSensor The sensor.
 * @return Whether the sensor has that type.
 */
bool isSensorBindable(const HitSensor* pSensor) {
    return pSensor->mSensorType == HitSensorType::Bindable;
}

/**
 * Checks whether a sensor belongs to a transparent block.
 * @param pSensor The sensor.
 * @return Whether the host is a transparent block.
 */
bool isSensorBlockTransparent(const HitSensor* pSensor) {
    LiveActor* host = pSensor->mHostActor;
    return isEqualString(host->getName(), "ロング透明ブロック") ||
           isEqualString(host->getName(), "透明ブロック");
}

/**
 * Checks whether a sensor belongs to a door key.
 * @param pSensor The sensor.
 * @return Whether the host is a door key.
 */
bool isSensorDoorKey(const HitSensor* pSensor) {
    return isSensorHostName(pSensor, "DoorKey");
}

/**
 * Checks whether a sensor belongs to a propeller box.
 * @param pSensor The sensor.
 * @return Whether the host is a propeller box.
 */
bool isSensorPropellerBox(const HitSensor* pSensor) {
    return isSensorHostName(pSensor, "プロペラボックス");
}

/**
 * Checks whether a sensor belongs to a goal item.
 * @param pSensor The sensor.
 * @return Whether the host is a goal item.
 */
bool isSensorGoalItem(const HitSensor* pSensor) {
    return isSensorHostName(pSensor, "GoalItem");
}

/**
 * Checks whether a sensor belongs to an empty goal item.
 * @param pSensor The sensor.
 * @return Whether the host is an empty goal item.
 */
bool isSensorGoalItemEmpty(const HitSensor* pSensor) {
    return isSensorHostName(pSensor, "GoalItemEmpty");
}

/**
 * Checks whether a sensor belongs to Plessie.
 * @param pSensor The sensor.
 * @return Whether the host is Plessie.
 */
bool isSensorPlessie(const HitSensor* pSensor) {
    return isEqualString(sead::SafeString(pSensor->mHostActor->getName()),
                         sead::SafeString("ライドン"));
}

/**
 * Checks whether a sensor belongs to a potted Piranha Plant.
 * @param pSensor The sensor.
 * @return Whether the host is a potted Piranha Plant.
 */
bool isSensorPackunWithPot(const HitSensor* pSensor) {
    LiveActor* host = pSensor->mHostActor;
    return isEqualString(host->getName(), "パックンフラワー（鉢植えあり）") ||
           isEqualString(host->getName(), "PackunFlowerWithPotFur") ||
           isEqualString(host->getName(), "PackunFlowerWithPot");
}

/**
 * Checks whether a sensor is any bindable sensor.
 * @param pSensor The sensor.
 * @return Whether the sensor is bindable.
 */
bool isSensorBindableAll(const HitSensor* pSensor) {
    return isSensorBindableGigaBell(pSensor) || isSensorBindableGoal(pSensor) ||
           isSensorBindableAllPlayer(pSensor) || isSensorBindableBubbleOutScreen(pSensor) ||
           isSensorBindableKoura(pSensor) || isSensorBindableRouteDokan(pSensor) ||
           isSensorBindableBubblePadInput(pSensor) || isSensorBindableGoalItem(pSensor) ||
           isSensorBindable(pSensor);
}

/**
 * Checks whether a sensor is a simple sensor.
 * @param pSensor The sensor.
 * @return Whether the sensor is simple.
 */
bool isSensorSimple(const HitSensor* pSensor) {
    if (pSensor->mSensorType == HitSensorType::EnemySimple ||
        pSensor->mSensorType == HitSensorType::MapObjSimple) {
        return true;
    }

    return isSensorBindable(pSensor);
}

/**
 * Updates the positions of all sensors of an actor.
 * @param pActor The actor.
 */
void updateHitSensorsAll(LiveActor* pActor) {
    pActor->mHitSensorKeeper->update();
}

/**
 * Checks whether a sensor belongs to an actor.
 * @param pSensor The sensor.
 * @param pActor The actor.
 * @return Whether the actor owns the sensor.
 */
bool isMySensor(const HitSensor* pSensor, const LiveActor* pActor) {
    return pSensor->mHostActor == pActor;
}

/**
 * Checks whether a sensor is closer to another one along a normal than the other's radius.
 * @param pSensor The sensor.
 * @param pPlane The sensor on the plane.
 * @param rNormal The plane normal.
 * @return Whether the sensor touches the plane.
 */
bool isSensorHitAnyPlane(const HitSensor* pSensor, const HitSensor* pPlane,
                         const sead::Vector3f& rNormal) {
    f32 radius = pPlane->mRadius;
    sead::Vector3f parallel;
    sead::Vector3f diff = pSensor->mPos - pPlane->mPos;
    parallelizeVec(&parallel, rNormal, diff);
    return parallel.squaredLength() < radius * radius;
}

/**
 * Checks whether a sensor is within the height of a ring around another sensor's host.
 * @param pSensor The sensor.
 * @param pRing The sensor of the ring.
 * @param width The ring width.
 * @return Whether the sensor touches the ring.
 */
bool isSensorHitRingShape(const HitSensor* pSensor, const HitSensor* pRing, f32 width) {
    sead::Matrix34f inv;
    inv.setInverse(*pRing->mHostActor->getBaseMtx());
    sead::Vector3f local;
    local.setMul(inv, pSensor->mPos);
    return sead::Mathf::abs(local.y) < width * 0.5f + pSensor->mRadius;
}

/**
 * Pushes a target sensor and removes the actor's velocity towards it.
 * @param pActor The actor.
 * @param pSelf The actor's sensor.
 * @param pTarget The target sensor.
 */
void sendMsgPushAndKillVelocityToTarget(LiveActor* pActor, HitSensor* pSelf, HitSensor* pTarget) {
    if (!sendMsgPush(pTarget, pSelf)) {
        return;
    }

    sead::Vector3f dir = pTarget->mPos - pSelf->mPos;
    if (normalizeOrZero(&dir)) {
        dir.e = sead::Vector3f::ez.e;
    }

    if (getVelocity(pActor).dot(dir) > 0.0f) {
        verticalizeVec(getVelocityPtr(pActor), dir, getVelocity(pActor));
    }
}

/**
 * Adds velocity to an actor to push it away from another sensor.
 * @param pActor The actor.
 * @param pOther The pushing sensor.
 * @param pSelf The actor's sensor.
 * @param speed The push speed.
 */
void pushAndAddVelocity(LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf,
                        f32 speed) {
    sead::Vector3f dir = pSelf->mPos - pOther->mPos;
    normalizeOrDirZ(&dir);
    f32 addSpeed = speed - getVelocity(pActor).dot(dir);
    if (addSpeed > 0.0f) {
        sead::Vector3f* velocity = getVelocityPtr(pActor);
        *velocity += addSpeed * dir;
    }
}

/**
 * Adds horizontal velocity to an actor to push it away from another sensor.
 * @param pActor The actor.
 * @param pOther The pushing sensor.
 * @param pSelf The actor's sensor.
 * @param speed The push speed.
 */
void pushAndAddVelocityH(LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf,
                         f32 speed) {
    sead::Vector3f dir = pSelf->mPos - pOther->mPos;
    dir.y = 0.0f;
    normalizeOrDirZ(&dir);
    f32 addSpeed = speed - getVelocity(pActor).dot(dir);
    if (addSpeed > 0.0f) {
        sead::Vector3f* velocity = getVelocityPtr(pActor);
        *velocity += addSpeed * dir;
    }
}

/**
 * Pushes an actor away from another sensor if the message is a push message.
 * @param pActor The actor.
 * @param pMsg The message.
 * @param pOther The pushing sensor.
 * @param pSelf The actor's sensor.
 * @param speed The push speed.
 * @return Whether the message was a push message.
 */
bool tryReceiveMsgPushAndAddVelocity(LiveActor* pActor, const SensorMsg* pMsg,
                                     const HitSensor* pOther, const HitSensor* pSelf, f32 speed) {
    if (!isMsgPush(pMsg) && !isMsgPushStrong(pMsg) && !isMsgPushVeryStrong(pMsg)) {
        return false;
    }

    pushAndAddVelocity(pActor, pOther, pSelf, speed);
    return true;
}

/**
 * Pushes an actor horizontally away from another sensor if the message is a push message.
 * @param pActor The actor.
 * @param pMsg The message.
 * @param pOther The pushing sensor.
 * @param pSelf The actor's sensor.
 * @param speed The push speed.
 * @return Whether the message was a push message.
 */
bool tryReceiveMsgPushAndAddVelocityH(LiveActor* pActor, const SensorMsg* pMsg,
                                      const HitSensor* pOther, const HitSensor* pSelf,
                                      f32 speed) {
    if (!isMsgPush(pMsg) && !isMsgPushStrong(pMsg) && !isMsgPushVeryStrong(pMsg)) {
        return false;
    }

    pushAndAddVelocityH(pActor, pOther, pSelf, speed);
    return true;
}

/**
 * Gets the type stored in a BindInit message.
 * @param pMsg The message.
 * @return The bind type.
 */
u32 getBindInitType(const SensorMsg* pMsg) {
    return sead::DynamicCast<const SensorMsgBindInit>(pMsg)->getType();
}

/**
 * Gets the type stored in a PlayerReleaseEquipmentGoal message.
 * @param pMsg The message.
 * @return The goal type.
 */
u32 getPlayerReleaseEquipmentGoalType(const SensorMsg* pMsg) {
    return sead::DynamicCast<const SensorMsgPlayerReleaseEquipmentGoal>(pMsg)->getType();
}

/**
 * Gets the tick recorded by a sensor.
 * @param pSensor The sensor.
 * @return The recorded tick.
 */
s64 getSensorTime(const HitSensor* pSensor) {
    return pSensor->mTime;
}

/**
 * Records the current tick in a sensor.
 * @param pSensor The sensor.
 */
void setSensorTime(HitSensor* pSensor) {
    pSensor->setTime();
}
}  // namespace al

namespace AttackSensorFunction {

/**
 * Gets the number of sensors a sensor hit.
 * @param pSensor The attacking sensor.
 * @return The number of hit sensors.
 */
s32 getAttackSensorNum(const al::HitSensor* pSensor) {
    return pSensor->mNumSensors;
}

/**
 * Gets a sensor hit by a sensor.
 * @param pSensor The attacking sensor.
 * @param idx The hit sensor index.
 * @return The hit sensor.
 */
al::HitSensor* getAttackSensor(const al::HitSensor* pSensor, s32 idx) {
    return pSensor->mSensors[idx];
}

/**
 * Finds the nearest sensor hit by a sensor.
 * @param pSensor The attacking sensor.
 * @return The nearest hit sensor, or nullptr if it hit nothing.
 */
al::HitSensor* findNearestAttackSensor(const al::HitSensor* pSensor) {
    al::HitSensor* nearest = nullptr;
    f32 nearestDist = 0.0f;
    u32 num = pSensor->mNumSensors;
    for (u32 i = 0; i != num; i++) {
        al::HitSensor* other = pSensor->mSensors[i];
        f32 dist = (other->mPos - pSensor->mPos).length();
        if (dist < nearestDist || nearest == nullptr) {
            nearestDist = dist;
            nearest = other;
        }
    }

    return nearest;
}
}  // namespace AttackSensorFunction

namespace al {

}  // namespace al
