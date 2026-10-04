#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class BlockRailRider;
    class ComboCounter;
    class HitSensor;
    class LiveActor;
    class SensorMsg;
};  // namespace al

namespace rc {
    bool sendMsgAskControlUserId(al::HitSensor* pSensor, s32 userId);
    bool sendMsgBlockRailRide(al::HitSensor* pReceiver, al::HitSensor* pSender,
                              al::BlockRailRider* pBlockRailRider);
    bool sendMsgCameraPush(al::HitSensor* pReceiver, al::HitSensor* pSender,
                           const sead::Vector3f& rPushVec);
    bool sendMsgDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time);
    bool sendMsgModifiedDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time);
    bool sendMsgFlingPoleDash(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 time);
    bool trySendMsgBlockToUpperObj(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 userId,
                                   al::ComboCounter* pComboCounter);
    bool trySendMsgBlockToLowerObj(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                   al::ComboCounter* pComboCounter);
    bool sendMsgAskBobsledDashPanel(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBobsledBodyAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBobsledTrample(al::HitSensor* pReceiver, al::HitSensor* pSender,
                               al::ComboCounter* pComboCounter);
    bool sendMsgBossGorobonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBossGorobonSpinShot(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBoxKillerBulletNoTouch(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgEnemyFloorTouchTrampoline(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGongShockwave(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGorobonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender,
                              al::ComboCounter* pComboCounter);
    bool sendMsgImozoTouch(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgDonketsuSlidePush(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgJumpPanelAction(al::HitSensor* pReceiver, al::HitSensor* pSender, bool isSuperJump);
    bool sendMsgKillerMagnumExplosion(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgKillerShockWave(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgKillerTouch(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPackunEat(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPackunEatStart(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPackunPush(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPackunThrowAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgItemReflect(al::HitSensor* pReceiver, al::HitSensor* pSender,
                            const sead::Vector3f& rHitDir);
    bool sendMsgPanelNoteHipDrop(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPlayerCheckpointTouch(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRouteDokanPlayerTouch(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                      const sead::Vector3f& rFront);
    bool sendMsgRouteDokanPlayerReflectNoDamage(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRouteDokanPlayerReflect(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRouteDokanItemGet(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRouteDokanPlayerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgSkateShoesAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgSpinnerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgTentackMagmaBallBreak(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgTuccondorAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgByugoWind(al::HitSensor* pReceiver, al::HitSensor* pSender,
                          const sead::Vector3f& rPower);
    bool sendMsgGustWind(al::HitSensor* pReceiver, al::HitSensor* pSender,
                         const sead::Vector3f& rPower);
    bool sendMsgBullAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRaidonAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgTakoboBulletAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgDossunPress(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGoalKillRunaway(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGroundSnapOffForce(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgAddForce(al::HitSensor* pReceiver, al::HitSensor* pSender,
                         const sead::Vector3f& rForce);
    bool sendMsgRequestTouchFromHoldedPlayer(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRingBeamerSign(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgItemBubbleBreak(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgItemBubbleBreakAndGetItem(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRouteDokanKouraAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBubbleAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgNokonokoKick(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRequestPlayerGetReaction(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                         const char* pName);
    bool sendMsgBombBoundKickedAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgBubbleVanish(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgIsEnableExitStage(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgIsEnableIslandWarp(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgIsDisableCancelBubble(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgQueryHostPlayer(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                const al::LiveActor* pHostPlayer);
    bool sendMsgNeedleRollerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgNeedleRollerHit(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                const sead::Vector3f& rDir, f32 power);
    bool sendMsgBoundTrampoline(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgMeraWanwanPush(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgMeraWanwanAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgKoopaLastBreakObj(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgKoopaLastReactionObj(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgStartGoalDemoPole(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGhostPresentGet(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgInkTouch(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPlayerGigaStep(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgFireRollerAttack(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgPushDir(al::HitSensor* pReceiver, al::HitSensor* pSender,
                        const sead::Vector3f& rDir);
    bool sendMsgPushConnected(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgGraffiti(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 type);
    bool sendMsgNpcBindInit(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgNpcBindCancel(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgRaidonBreakLightReaction(al::HitSensor* pReceiver, al::HitSensor* pSender);
    bool sendMsgNeedleRollerHitToCollition(al::LiveActor* pActor, al::HitSensor* pSender,
                                           const sead::Vector3f& rDir, f32 power);
    al::BlockRailRider* tryGetMsgParamBlockRailRider(const al::SensorMsg* pMsg);
    bool isMsgAskControlUserId(const al::SensorMsg* pMsg, s32 userId);
    bool isMsgAskControlUserId(const al::SensorMsg* pMsg, const al::HitSensor* pSensor);
    bool isMsgBlockRailRide(const al::SensorMsg* pMsg);
    bool isMsgCameraPush(const al::SensorMsg* pMsg);
    bool isMsgDashPanel(const al::SensorMsg* pMsg);
    bool isMsgModifiedDashPanel(const al::SensorMsg* pMsg);
    bool isMsgFlingPoleDash(const al::SensorMsg* pMsg);
    bool isMsgCoinGet(const al::SensorMsg* pMsg);
    bool isMsgAskBobsledDashPanel(const al::SensorMsg* pMsg);
    bool isMsgBobsledBodyAttack(const al::SensorMsg* pMsg);
    bool isMsgBobsledTrample(const al::SensorMsg* pMsg);
    bool isMsgBossGorobonAttack(const al::SensorMsg* pMsg);
    bool isMsgBossGorobonSpinShot(const al::SensorMsg* pMsg);
    bool isMsgBoxKillerBulletNoTouch(const al::SensorMsg* pMsg);
    bool isMsgEnemyFloorTouchTrampoline(const al::SensorMsg* pMsg);
    bool isMsgGongShockwave(const al::SensorMsg* pMsg);
    bool isMsgGorobonAttack(const al::SensorMsg* pMsg);
    bool isMsgImozoTouch(const al::SensorMsg* pMsg);
    bool isMsgDonketsuSlidePush(const al::SensorMsg* pMsg);
    bool isMsgItemReflect(const al::SensorMsg* pMsg);
    bool isMsgJumpPanelAction(const al::SensorMsg* pMsg);
    bool isMsgKillerMagnumExplosion(const al::SensorMsg* pMsg);
    bool isMsgKillerShockWave(const al::SensorMsg* pMsg);
    bool isMsgKillerTouch(const al::SensorMsg* pMsg);
    bool isMsgPackunEat(const al::SensorMsg* pMsg);
    bool isMsgPackunEatStart(const al::SensorMsg* pMsg);
    bool isMsgPackunPush(const al::SensorMsg* pMsg);
    bool isMsgPackunThrowAttack(const al::SensorMsg* pMsg);
    bool isMsgPanelNoteHipDrop(const al::SensorMsg* pMsg);
    bool isMsgPlayerCheckpointTouch(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanPlayerAttack(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanPlayerTouch(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanPlayerReflect(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanPlayerReflectNoDamage(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanItemGet(const al::SensorMsg* pMsg);
    bool isMsgSpinnerAttack(const al::SensorMsg* pMsg);
    bool isMsgSkateShoesAttack(const al::SensorMsg* pMsg);
    bool isMsgTentackMagmaBallBreak(const al::SensorMsg* pMsg);
    bool isMsgTuccondorAttack(const al::SensorMsg* pMsg);
    bool isMsgByugoWind(const al::SensorMsg* pMsg);
    bool isMsgBullAttack(const al::SensorMsg* pMsg);
    bool isMsgRaidonAttack(const al::SensorMsg* pMsg);
    bool isMsgTakoboBulletAttack(const al::SensorMsg* pMsg);
    bool isMsgDossunPress(const al::SensorMsg* pMsg);
    bool isMsgGoalKillRunaway(const al::SensorMsg* pMsg);
    bool isMsgGroundSnapOffForce(const al::SensorMsg* pMsg);
    bool isMsgAddForce(const al::SensorMsg* pMsg);
    bool isMsgRequestTouchFromHoldedPlayer(const al::SensorMsg* pMsg);
    bool isMsgRingBeamerSign(const al::SensorMsg* pMsg);
    bool isMsgItemBubbleBreak(const al::SensorMsg* pMsg);
    bool isMsgItemBubbleBreakAndGetItem(const al::SensorMsg* pMsg);
    bool isMsgRouteDokanKouraAttack(const al::SensorMsg* pMsg);
    bool isMsgBubbleAttack(const al::SensorMsg* pMsg);
    bool isMsgNokonokoKick(const al::SensorMsg* pMsg);
    bool isMsgRequestPlayerGetReaction(const al::SensorMsg* pMsg);
    bool isMsgBombBoundKickedAttack(const al::SensorMsg* pMsg);
    bool isMsgBubbleVanish(const al::SensorMsg* pMsg);
    bool isMsgTouchAssistBurnPeto(const al::SensorMsg* pMsg);
    bool isMsgIsEnableExitStage(const al::SensorMsg* pMsg);
    bool isMsgIsEnableIslandWarp(const al::SensorMsg* pMsg);
    bool isMsgIsDisableCancelBubble(const al::SensorMsg* pMsg);
    bool isMsgQueryHostPlayer(const al::SensorMsg* pMsg);
    bool isMsgNeedleRollerAttack(const al::SensorMsg* pMsg);
    bool isMsgNeedleRollerHit(const al::SensorMsg* pMsg);
    bool isMsgBoundTrampoline(const al::SensorMsg* pMsg);
    bool isMsgMeraWanwanPush(const al::SensorMsg* pMsg);
    bool isMsgMeraWanwanAttack(const al::SensorMsg* pMsg);
    bool isMsgKoopaLastBreakObj(const al::SensorMsg* pMsg);
    bool isMsgKoopaLastReactionObj(const al::SensorMsg* pMsg);
    bool isMsgStartGoalDemoPole(const al::SensorMsg* pMsg);
    bool isMsgStartGoalDemoHouse(const al::SensorMsg* pMsg);
    bool isMsgStartDemoBossStart(const al::SensorMsg* pMsg);
    bool isMsgGhostPresentGet(const al::SensorMsg* pMsg);
    bool isMsgBindInitNormal(const al::SensorMsg* pMsg);
    bool isMsgBindInitRequest(const al::SensorMsg* pMsg);
    bool isMsgBindInitGiant(const al::SensorMsg* pMsg);
    bool isMsgInkTouch(const al::SensorMsg* pMsg);
    bool isMsgPlayerGigaStep(const al::SensorMsg* pMsg);
    bool isMsgPushDir(const al::SensorMsg* pMsg);
    bool isMsgPushConnected(const al::SensorMsg* pMsg);
    bool isMsgGraffiti(const al::SensorMsg* pMsg);
    bool isMsgFireRollerAttack(const al::SensorMsg* pMsg);
    bool isMsgNpcBindInit(const al::SensorMsg* pMsg);
    bool isMsgNpcBindCancel(const al::SensorMsg* pMsg);
    bool isMsgRaidonBreakLightReaction(const al::SensorMsg* pMsg);
    bool isSensorKinopioBrigadeNpc(const al::HitSensor* pSensor);
    al::ComboCounter* getMsgComboCount(const al::SensorMsg* pMsg);
    al::ComboCounter* tryGetMsgComboCount(const al::SensorMsg* pMsg);
    bool tryGetDashPanelTime(s32* pTime, const al::SensorMsg* pMsg);
    bool tryGetWindPower(sead::Vector3f* pPower, const al::SensorMsg* pMsg);
    bool tryGetItemReflectHitDir(sead::Vector3f* pHitDir, const al::SensorMsg* pMsg);
    bool tryGetNeedleRollerHitParam(sead::Vector3f* pDir, f32* pPower, const al::SensorMsg* pMsg);
    bool tryGetRouteDokanPlayerTouchFront(sead::Vector3f* pFront, const al::SensorMsg* pMsg);
    bool tryGetForce(sead::Vector3f* pForce, const al::SensorMsg* pMsg);
    bool isEqualHostPlayer(const al::SensorMsg* pMsg, const al::LiveActor* pPlayer);
    bool tryGetRequestPlayerGetReactionName(const char** pName, const al::SensorMsg* pMsg);
    bool tryRelayRequestPlayerGetReactionMsg(const al::SensorMsg* pMsg, al::HitSensor* pSender,
                                             al::HitSensor* pReceiver);
    bool sendMsgCameraPushToPlayer(al::LiveActor* pPlayer, al::HitSensor* pSender,
                                   const sead::Vector3f& rPushVec);
    void startHitReactionBlowHitMessage(const al::SensorMsg* pMsg, const al::LiveActor* pActor,
                                        const al::HitSensor* pOther, const al::HitSensor* pSelf);
    void startHitReactionBlowHitMessage(const al::SensorMsg* pMsg, const al::LiveActor* pActor);
    void requestHitReactionToAttacker(const al::SensorMsg* pMsg, const al::HitSensor* pOther,
                                      const al::HitSensor* pSelf);
    void requestHitReactionToAttackerNpc(const al::HitSensor* pOther, const al::HitSensor* pSelf);
    void requestHitReactionToAttacker(const char* pName, const al::HitSensor* pOther,
                                      const al::HitSensor* pSelf);
    bool sendMsgDebugMovePosition(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                  const sead::Vector3f& rPos);
    bool tryGetDebugMovePosition(sead::Vector3f* pPos, const al::SensorMsg* pMsg);
    bool tryGetPushDir(sead::Vector3f* pDir, const al::SensorMsg* pMsg);
    bool tryReceiveMsgPushDirAndAddVelocity(al::LiveActor* pActor, const al::SensorMsg* pMsg,
                                            f32 speed);
    bool tryReceiveMsgPushConnectedAndAddVelocity(al::LiveActor* pActor, const al::SensorMsg* pMsg,
                                                  const al::HitSensor* pOther,
                                                  const al::HitSensor* pSelf, f32 speed);
    bool isMsgJumpPanelActionAndSuperJump(const al::SensorMsg* pMsg);
};  // namespace rc
