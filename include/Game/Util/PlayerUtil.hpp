#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Player/PlayerDef.hpp"

namespace al {
    class BossPlayReportTask;
    class HitSensor;
    class LiveActor;
    class NetworkSystem;
    class PlayerHolder;
    class Triangle;
};

namespace sead {
    class IDelegate;
};

class GameDataHolder;
class IUsePlayerKeyConfig;
class PlayerActor;
class PlayerAliveWatcher;

namespace rc {
    const char* getPlayerParamsName();
    bool toggleUsingOldPlayerParams();
    void setUsingOldPlayerParams(bool isOld);
    bool isUsingOldPlayerParams();
    const char* getPlayerInputName();
    void toggleUsingHoldInput(bool isUseInput);
    bool isReallyPlayerActor(const al::LiveActor* pActor);
    bool isReallyPlayerActor(const al::HitSensor* pSensor);
    void doGigaHitReaction(const al::LiveActor* pActor, const char* pName);
    void doGigaHitReaction(const al::LiveActor* pActor, const char* pName,
                           const sead::Vector3f& rPos);
    s32 getPlayerCharacterNumMax();
    s32 getPlayerCharacterNumMaxTrue();
    const char* getPlayerCharacterName(s32 characterType);
    const char* getPlayerCharacterName(const al::LiveActor* pActor);
    s32 getPlayerCharaType(const al::LiveActor* pActor);
    const char* getPlayerCharacterNameTrue(s32 characterType);
    const char* getPlayerCharacterNameTrue(const al::LiveActor* pActor);
    const char* getPlayerCharacterBubbleMatAnimName(s32 characterType);
    const char* getPlayerCharacterBubbleMatAnimName(const al::LiveActor* pActor);
    s32 getPlayerFigureNumMax();
    s32 getPlayerFigureTypeDefault();
    void invalidatePlayerInput(const al::LiveActor* pActor, s32 frame);
    bool isAnyPlayerJumpTrigOn(const al::LiveActor* pActor);
    bool isPlayerDeadOrBubble(const al::LiveActor* pActor);
    bool isPlayerJumpTrigOn(const al::LiveActor* pActor);
    bool isPlayerJumpTrigOn(const al::LiveActor* pActor, s32 index);
    bool isPlayerInputJumpTrigOn(const al::LiveActor* pActor, bool isButton);
    void requestHideBubbledPlayers(const al::LiveActor* pActor);
    bool isPlayerBubble(const al::LiveActor* pActor);
    void requestBindAllPlayer(const al::LiveActor* pActor, al::HitSensor* pSensor);
    void setDisableReviveBubbleForAllPlayer(al::LiveActor* pActor);
    void requestBindAllPlayerButBubble(const al::LiveActor* pActor, al::HitSensor* pSensor);
    void requestBindAllPlayerButBubbleExcludeActor(const al::LiveActor* pActor,
                                                   al::HitSensor* pSensor,
                                                   const al::LiveActor* pExcludeActor);
    void requestBindAllPlayerAcceptReviveBubble(const al::LiveActor* pActor,
                                                al::HitSensor* pSensor);
    void cancelRequestBindAllPlayer(const al::LiveActor* pActor, al::HitSensor* pSensor);
    bool isAllPlayerBinded(const al::LiveActor* pActor, al::HitSensor* pSensor);
    bool isAllPlayerBinded(const al::LiveActor* pActor);
    bool checkAllPlayerBindedAndDisableReviveBubble(const al::LiveActor* pActor,
                                                    al::HitSensor* pSensor);
    bool checkAllPlayerBindedOrBubbleAndDisableReviveBubble(const al::LiveActor* pActor,
                                                            al::HitSensor* pSensor);
    bool checkAllPlayerBindedOrBubbleAndDisableReviveBubbleExcludeActor(const al::LiveActor* pActor,
                                                                        al::HitSensor* pSensor,
                                                                        const al::LiveActor* pExcludeActor);
    void hideAllDyingPlayers(const al::LiveActor* pActor);
    bool isPlayerInvincible(const al::LiveActor* pActor, s32 index);
    bool isPlayerInvincible(const al::LiveActor* pActor, const al::HitSensor* pSensor);
    bool isPlayerInvincible(const al::LiveActor* pActor);
    void tryPauseAllPlayerInvincible(const al::LiveActor* pActor, bool isPauseBgm);
    void tryPauseAllPlayerInvincible(al::PlayerHolder* pHolder, bool isPauseBgm);
    void tryPausePlayerInvincible(al::LiveActor* pActor, bool isPauseBgm);
    void tryResumeAllPlayerInvincible(const al::LiveActor* pActor, bool isResumeBgm);
    void tryResumeAllPlayerInvincible(al::PlayerHolder* pHolder, bool isResumeBgm);
    void tryResumePlayerInvincible(al::LiveActor* pActor, bool isResumeBgm);
    void trySetPlayerInvicibleBgmState(al::HitSensor* pSensor, bool isOn);
    bool isAllPlayerDeadOrBubble(const al::LiveActor* pActor);
    bool isAllPlayerOnGround(const al::LiveActor* pActor);
    bool isPlayerOnGround(const al::LiveActor* pActor);
    bool isAllPlayerOnGroundOrBubble(const al::LiveActor* pActor);
    bool isAllPlayerOnGroundOrWater(const al::LiveActor* pActor);
    bool isPlayerInWater(const al::LiveActor* pActor);
    bool isAllPlayerOnGroundOrWaterOrKoura(const al::LiveActor* pActor);
    bool isAllPlayerOnGroundNoDeadOrBubble(const al::LiveActor* pActor);
    bool isAnyPlayerOnGroundNoDeadOrBubble(const al::LiveActor* pActor);
    bool isPlayerAbyss(const al::LiveActor* pActor);
    bool isPlayerVanishDying(al::LiveActor* pActor);
    void setPlayerVanishDying(al::LiveActor* pActor);
    bool isPlayerInRouteDokanOrDokan(const al::LiveActor* pActor);
    bool isPlayerInDokanNotRouteDokan(const al::LiveActor* pActor);
    bool isPlayerInRouteDokanSM(const al::LiveActor* pActor);
    bool isPlayerInRouteDokan(const al::LiveActor* pActor);
    bool isPlayerInRouteDokan(const al::HitSensor* pSensor);
    bool isPlayerInRouteDokanBazooka(const al::LiveActor* pActor);
    bool isPlayerInRouteDokanBazooka(const al::HitSensor* pSensor);
    bool isPlayerOnRaidon(const al::LiveActor* pActor);
    bool isPlayerOnRaidon(const al::HitSensor* pSensor);
    bool isPlayerOnRaidonGround(const al::LiveActor* pActor);
    bool isPlayerOnSkateShoes(const al::LiveActor* pActor);
    bool isPlayerOnSkateShoesGround(const al::LiveActor* pActor);
    bool isPlayerDamageInvalid(const al::LiveActor* pActor);
    bool isPlayerDamageInvalid(const al::HitSensor* pSensor);
    bool isPlayerInInkLimiter(const al::LiveActor* pActor);
    bool isPlayerInCloudBonus(const al::LiveActor* pActor);
    bool isPlayerWait(const al::LiveActor* pActor);
    bool isPlayerDash(const al::LiveActor* pActor);
    bool isPlayerDash(const al::HitSensor* pSensor);
    bool isPlayerDashFast(const al::LiveActor* pActor);
    bool isPlayerGreaterSuperDashMaxSpeed(const al::HitSensor* pSensor);
    bool isPlayerSquat(const al::LiveActor* pActor);
    bool isPlayerSquat(const al::HitSensor* pSensor);
    bool isPlayerTurn(const al::LiveActor* pActor);
    bool isPlayerDead(const al::LiveActor* pActor);
    bool isPlayerEnableBubble(const al::LiveActor* pActor, const al::HitSensor* pSensor);
    bool isPlayerDeadOrBubble(const al::LiveActor* pActor, s32 index);
    bool isPlayingDeadAnim(const al::LiveActor* pActor);
    void forceEndPlayerDeadAnim(const al::LiveActor* pActor);
    bool isPlayerOnGround(const al::HitSensor* pSensor);
    bool isPlayerOnGroundOrWater(const al::LiveActor* pActor);
    bool isPlayerOnGroundOrWater(const al::HitSensor* pSensor);
    bool isPlayerOnGround(const al::LiveActor* pActor, s32 index);
    bool isPlayerBinded(const al::LiveActor* pActor);
    bool isPlayerInKouraOnGround(const al::LiveActor* pActor);
    bool isPlayerInKoura(const al::LiveActor* pActor);
    void cancelPlayerInKoura(const al::LiveActor* pActor);
    al::HitSensor* getPlayerBindedSenser(al::LiveActor* pActor);
    bool isPlayerBinded(const al::HitSensor* pSensor);
    bool isPlayerBinded(const al::LiveActor* pActor, s32 index);
    bool isPlayerInWater(const al::HitSensor* pSensor);
    bool isPlayerInWaterSurface(const al::HitSensor* pSensor);
    bool isPlayerHipDropping(const al::HitSensor* pSensor);
    bool isPlayerHipDropping(const al::LiveActor* pActor);
    bool isPlayerHipDropping(const al::LiveActor* pActor, s32 index);
    bool isPlayerRollingOnGround(const al::HitSensor* pSensor);
    bool isPlayerSliding(const al::LiveActor* pActor);
    bool isPlayerSliding(const al::LiveActor* pActor, s32 index);
    bool isPlayerDamageTrigOn(const al::HitSensor* pSensor);
    bool isPlayerDamageTrigOn(const al::LiveActor* pActor);
    bool isPlayerWallSnap(const al::HitSensor* pSensor);
    bool isPlayerWallSnap(const al::LiveActor* pActor);
    bool isPlayerSpinGround(const al::LiveActor* pActor);
    bool isPlayerSpinJump(const al::LiveActor* pActor);
    bool isPlayerBodyAttack(const al::LiveActor* pActor);
    bool isPlayerClimbGigaBodyAttack(const al::HitSensor* pSensor);
    bool isPlayerSkating(const al::LiveActor* pActor);
    bool isPlayerOnColliderGround(const al::LiveActor* pActor, const al::LiveActor* pFloorActor);
    al::LiveActor* getActivePlayer(al::PlayerHolder* pHolder);
    al::LiveActor* getActivePlayer(const al::LiveActor* pActor);
    bool isPlayerChangeDemoAny(const al::PlayerHolder* pHolder);
    bool isPlayerChangeDemoAny(const al::LiveActor* pActor);
    bool isPlayerChara(const al::LiveActor* pActor, EPlayerChara chara);
    bool isPlayerChara(const al::HitSensor* pSensor, EPlayerChara chara);
    bool isPlayerCharaMario(const al::HitSensor* pSensor);
    bool isPlayerCharaLuigi(const al::HitSensor* pSensor);
    bool isPlayerCharaPeach(const al::HitSensor* pSensor);
    bool isPlayerCharaKinopio(const al::HitSensor* pSensor);
    bool isPlayerCharaRosetta(const al::HitSensor* pSensor);
    s32 getPlayerCharaType(const al::HitSensor* pSensor);
    void tryChangeToSuperMario(const al::HitSensor* pSensor);
    void tryChangeToSuperMario(al::LiveActor* pActor);
    void tryChangeToSuperMario(al::LiveActor* pActor, s32 userId);
    al::LiveActor* findPlayerActorFirstByUserId(const al::LiveActor* pActor, s32 userId);
    void tryChangeHoldedPlayerToSuperMario(const al::HitSensor* pSensor);
    void tryChangeToFireMario(const al::HitSensor* pSensor);
    void tryChangeToClimbMario(const al::HitSensor* pSensor);
    void tryChangeToRaccoonDogMario(const al::HitSensor* pSensor);
    void tryChangeToBoomerangMario(const al::HitSensor* pSensor);
    void tryChangeToInvincibleMario(const al::HitSensor* pSensor);
    void tryChangeToInvincibleMario(al::LiveActor* pActor);
    void tryChangeToInvincibleAllPlayer(const al::LiveActor* pActor, bool isPlayBgm);
    void cancelInvincibleMarioForce(const al::HitSensor* pSensor, bool isStopBgm, bool isKeepModel);
    void cancelInvincibleMarioForce(al::LiveActor* pActor, bool isStopBgm, bool isKeepModel);
    void tryChangeToGiantMario(const al::HitSensor* pSensor);
    void changeToSuperMarioForce(const al::HitSensor* pSensor);
    bool isPlayerEquipHeadgear(const al::HitSensor* pSensor);
    void removePlayerEquipHeadgear(const al::HitSensor* pSensor, bool isGoal);
    void tryChangeToGiantMario(al::LiveActor* pActor);
    void changeToSuperMarioForce(al::LiveActor* pActor);
    bool isPlayerEquipHeadgear(const al::LiveActor* pActor);
    void removePlayerEquipHeadgear(const al::LiveActor* pActor, bool isGoal);
    void tryChangeToGigaMario(const al::LiveActor* pBell, const al::HitSensor* pSensor);
    void tryChangeToGigaMario(const al::LiveActor* pBell, al::LiveActor* pActor);
    void tryChangeToGigaClimbMario(const al::LiveActor* pBell, const al::HitSensor* pSensor);
    void tryChangeToGigaClimbMario(const al::LiveActor* pBell, al::LiveActor* pActor, bool isForce);
    bool isPlayerGiga(const al::LiveActor* pActor);
    bool isPlayerGiga(al::PlayerHolder* pHolder);
    f32 getGigaScaleRate(const al::LiveActor* pActor);
    void setStayInGigaMario(al::LiveActor* pActor, bool isResetEndTimer);
    void unsetStayInGigaMario(al::LiveActor* pActor, bool isResetEndTimer);
    void restartGigaMarioGraph(al::LiveActor* pActor);
    void cancelGiantMario(const al::HitSensor* pSensor);
    void cancelGiantMario(al::LiveActor* pActor);
    void cancelGiantMarioForce(const al::HitSensor* pSensor);
    void cancelGiantMarioForce(al::LiveActor* pActor);
    void tryChangeToRaccoonDogWhiteMario(const al::HitSensor* pSensor);
    void tryChangeToClimbMarioSpecial(const al::HitSensor* pSensor);
    void tryChangeToClimbWhiteMario(const al::HitSensor* pSensor);
    void tryChangeToClimbGigaMario(const al::HitSensor* pSensor);
    void changeToMiniMarioForce(al::LiveActor* pActor);
    void changeToClimbMarioForce(al::LiveActor* pActor);
    void changeToRaccoonDogMarioForce(al::LiveActor* pActor);
    void changeToFireMarioForce(al::LiveActor* pActor);
    void changeToBoomerangMarioForce(al::LiveActor* pActor);
    void changeToRaccoonDogWhiteMarioForce(al::LiveActor* pActor);
    void changeToClimbMarioSpecialForce(al::LiveActor* pActor);
    void changeToClimbWhiteMarioForce(al::LiveActor* pActor);
    void changeToClimbGigaMarioForce(al::LiveActor* pActor);
    void initPlayerFigureType(al::LiveActor* pActor, u32 figure, bool isForce);
    s32 getPlayerFigureType(const al::LiveActor* pActor);
    const char* getPlayerFigureName(al::LiveActor* pActor);
    const char* getPlayerRealFigureName(al::LiveActor* pActor);
    s32 getPlayerFigureTypeWithNext(const al::LiveActor* pActor);
    bool isPlayerSuper(const al::HitSensor* pSensor);
    bool isPlayerMini(const al::HitSensor* pSensor);
    bool isPlayerMini(al::LiveActor* pActor);
    bool isPlayerClimb(const al::HitSensor* pSensor);
    bool isPlayerClimb(const al::LiveActor* pActor);
    bool isPlayerRaccoonDog(const al::HitSensor* pSensor);
    bool isPlayerFire(const al::HitSensor* pSensor);
    bool isPlayerBoomerang(const al::HitSensor* pSensor);
    bool isPlayerGiant(const al::HitSensor* pSensor);
    bool isPlayerGiant(const al::LiveActor* pActor);
    bool isPlayerRaccoonDogWhite(const al::HitSensor* pSensor);
    bool isPlayerRaccoonDogWhite(const al::LiveActor* pActor);
    bool isPlayerClimbSpecial(const al::HitSensor* pSensor);
    bool isPlayerClimbSpecial(const al::LiveActor* pActor);
    bool isPlayerClimbOrClimbSpecial(const al::HitSensor* pSensor);
    bool isPlayerClimbWhite(const al::HitSensor* pSensor);
    bool isPlayerClimbGiga(const al::HitSensor* pSensor);
    bool isPlayerClimbOrClimbSpecial(const al::LiveActor* pActor);
    bool isPlayerClimbWhite(const al::LiveActor* pActor);
    bool isPlayerClimbGiga(const al::LiveActor* pActor);
    bool isPlayerManekinekoStatueOn(const al::HitSensor* pSensor);
    bool isPlayerManekinekoStatueOn(const al::LiveActor* pActor);
    bool isPlayerManekinekoStatueOn(const al::LiveActor* pActor, s32 index);
    bool isPlayerNextFigureRequested(const al::LiveActor* pActor);
    s32 getPlayerNextFigureType(const al::LiveActor* pActor);
    bool isAnyPlayerMini(const al::LiveActor* pActor);
    void revivePlayer(const al::HitSensor* pSensor);
    void revivePlayer(al::LiveActor* pActor);
    void resetDisableReviveBubbleForAllPlayer(al::LiveActor* pActor);
    void setDisableFrameOutBubbleForAllPlayer(al::LiveActor* pActor);
    void resetDisableFrameOutBubbleForAllPlayer(al::LiveActor* pActor);
    void cancelRequestBindAndResetDisableReviveBubbleForAllPlayer(al::LiveActor* pActor,
                                                                  al::HitSensor* pSensor);
    void forceKillPlayer(al::LiveActor* pActor);
    void tryForceKillPlayer(al::LiveActor* pActor);
    const sead::Vector3f& getPlayerFront(const al::LiveActor* pActor);
    const sead::Vector3f& getPlayerFront(const al::HitSensor* pSensor);
    const sead::Vector3f& getPlayerVelocity(const al::HitSensor* pSensor);
    const sead::Vector3f& getPlayerVelocity(const al::LiveActor* pActor);
    const sead::Vector3f& getPlayerVelocity(const al::LiveActor* pActor, s32 index);
    f32 getPlayerSpeedH(const al::HitSensor* pSensor);
    f32 getPlayerSpeedH(const al::LiveActor* pActor, s32 index);
    const sead::Matrix34f* getPlayerViewMtx(const al::HitSensor* pSensor);
    s32 getPlayerInputPort(const al::HitSensor* pSensor);
    s32 getPlayerInputPort(const al::LiveActor* pActor);
    s32 getPlayerInputPort(const al::LiveActor* pActor, s32 index);
    al::LiveActor* findPlayerFromInputPort(const al::PlayerHolder* pHolder, s32 port);
    al::LiveActor* tryFindPlayerFromInputPort(const al::PlayerHolder* pHolder, s32 port,
                                              bool isAcceptBubble);
    al::LiveActor* findPlayerFromInputPort(const al::LiveActor* pActor, s32 port);
    al::LiveActor* tryFindPlayerFromInputPort(const al::LiveActor* pActor, s32 port,
                                              bool isAcceptBubble);
    void calcPlayerHeadColliderPos(sead::Vector3f* pPos, const al::LiveActor* pActor);
    void calcPlayerBodyColliderPos(sead::Vector3f* pPos, const al::LiveActor* pActor);
    void calcPlayerSide(sead::Vector3f* pSide, const al::LiveActor* pActor);
    void trySetPlayerForwardToInput(PlayerActor* pPlayer);
    void trySetPlayerForwardToInput(al::LiveActor* pActor);
    void trySetPlayerForwardToInput(al::HitSensor* pSensor);
    bool isPlayerHideModel(const al::HitSensor* pSensor);
    bool isPlayerHideModel(const al::LiveActor* pActor);
    bool isPlayerHideSilhouette(const al::HitSensor* pSensor);
    bool isPlayerHideSilhouette(const al::LiveActor* pActor);
    sead::Matrix34f* getPlayerModelJointMtxPtr(const al::HitSensor* pSensor,
                                               const char* pJointName);
    sead::Matrix34f* getPlayerModelJointMtxPtr(const al::LiveActor* pActor, const char* pJointName);
    void calcPlayerModelJointPos(sead::Vector3f* pPos, const al::HitSensor* pSensor,
                                 const char* pJointName);
    void calcPlayerHoldPos(sead::Vector3f* pPos, const al::HitSensor* pSensor);
    void calcPlayerHoldMtx(sead::Matrix34f* pMtx, const al::HitSensor* pSensor);
    const IUsePlayerKeyConfig* getPlayerKeyConfig(const al::HitSensor* pSensor);
    const IUsePlayerKeyConfig* getPlayerKeyConfig(const al::LiveActor* pActor);
    bool tryPlayerEquipHeadgear(const al::HitSensor* pSensor, al::HitSensor* pHeadgearSensor,
                                sead::IDelegate* pDelegate, u32 action);
    void removePlayerEquipHeadgearSilent(const al::HitSensor* pSensor);
    void removePlayerEquipHeadgearSilent(const al::LiveActor* pActor);
    bool tryPlayerEquipCrown(const al::HitSensor* pSensor, al::HitSensor* pCrownSensor, u32 action);
    void removePlayerEquipCrown(const al::HitSensor* pSensor);
    void removePlayerEquipCrownGoalPole(const al::HitSensor* pSensor);
    void removePlayerEquipCrownGateKeeper(const al::HitSensor* pSensor);
    bool isPlayerPropellerRising(const al::HitSensor* pSensor);
    bool isPlayerPropellerJumping(const al::HitSensor* pSensor);
    bool isPlayerPropellerGlide(const al::HitSensor* pSensor);
    bool isPlayerEquipDisregard(const al::LiveActor* pActor);
    void removeAllEquipFromPlayer(const al::HitSensor* pSensor);
    void removeAllEquipFromPlayer(al::LiveActor* pActor);
    void removeAllEquipFromPlayerGoalPole(const al::HitSensor* pSensor);
    void removeAllEquipFromPlayerGoalPole(al::LiveActor* pActor);
    void removeAllEquipFromPlayerGateKeeper(const al::HitSensor* pSensor);
    void removeAllEquipFromPlayerGateKeeper(al::LiveActor* pActor);
    void removeAllEquipFromPlayerSilent(const al::HitSensor* pSensor);
    void removeAllEquipFromPlayerSilent(al::LiveActor* pActor);
    void pausePlayerEquip(al::LiveActor* pActor);
    void resumePlayerEquip(al::LiveActor* pActor);
    void requestPlayerRelease(al::HitSensor* pSensor);
    bool isPlayerHolded(const al::HitSensor* pSensor);
    bool isPlayerHoldingAnotherPlayer(const al::HitSensor* pSensor);
    bool isPlayerHoldingSomething(const al::HitSensor* pSensor);
    bool isPlayerHolding(const al::HitSensor* pSensor, const al::LiveActor* pHoldActor);
    bool isPlayerHolding(al::LiveActor* pActor, s32 userId, const al::HitSensor* pSensor);
    al::LiveActor* getPlayerHoldingActor(const al::LiveActor* pActor);
    void hidePlayerHoldingItem(al::HitSensor* pSensor, bool isHide, bool isHideShadow);
    void hidePlayerHoldingItem(al::LiveActor* pActor, bool isHide, bool isHideShadow);
    void cancelSinkSe(al::HitSensor* pSensor);
    void cancelSinkSe(al::LiveActor* pActor);
    void validatePlayerGetItem(al::HitSensor* pSensor);
    void invalidatePlayerGetItem(al::HitSensor* pSensor);
    void requestPlayerBindWithPriority(al::HitSensor* pSensor, al::HitSensor* pBinderSensor);
    void requestPlayerBind(al::HitSensor* pSensor, al::HitSensor* pBinderSensor);
    void requestPlayerBindTractorBubble(al::LiveActor* pActor, al::HitSensor* pBubbleSensor);
    bool isPlayerBinded(al::HitSensor* pSensor, al::HitSensor* pBinderSensor);
    bool isPlayerEnableBind(al::HitSensor* pSensor, const al::HitSensor* pBinderSensor);
    void appendDoubleMario(PlayerActor* pPlayer);
    void calcDoubleMarioCenterPos(sead::Vector3f* pPos, const PlayerActor* pPlayer);
    void calcDoubleMarioHeadPos(sead::Vector3f* pPos, const PlayerActor* pPlayer,
                                const sead::Vector3f& rOffset);
    const sead::Vector3f& getPlayerTrans(const PlayerActor* pPlayer);
    bool isLastDoubleMario(const PlayerActor* pPlayer);
    void killAllDoubleMarioExcept(PlayerActor* pPlayer);
    void killAllDoubleMarioExcept(al::HitSensor* pSensor);
    void killAllDoubleMarioExceptWithScore(al::HitSensor* pSensor);
    s32 calcDoubleMarioTotalNum(const al::LiveActor* pActor);
    void activatePlayerWithBubble(PlayerActor* pPlayer, s32 port, const sead::Vector3f* pTrans,
                                  const sead::Vector3f* pFront);
    void activatePlayer(PlayerActor* pPlayer, s32 port, const sead::Vector3f* pTrans,
                        const sead::Vector3f* pFront);
    void deactivatePlayer(PlayerActor* pPlayer);
    void invalidatePlayerDamagePipe(PlayerActor* pPlayer);
    void validatePlayerDamagePipe(PlayerActor* pPlayer);
    void invalidatePlayerDamage(PlayerActor* pPlayer, u32 frame);
    void invalidatePlayerFlash(PlayerActor* pPlayer);
    void validatePlayerFlash(PlayerActor* pPlayer);
    void removeAllEquipOfAllDoubleMarioExceptWithScore(al::HitSensor* pSensor);
    void setMainPlayerActor(al::LiveActor* pActor);
    bool calcPlayerDotMinMax(f32* pMin, f32* pMax, const al::LiveActor* pActor,
                             const sead::Vector3f& rOrigin, const sead::Vector3f& rDir);
    al::LiveActor* tryFindNearestActivePlayerActorInSphere(const al::LiveActor* pActor, f32 radius);
    al::LiveActor* tryFindNearestActivePlayerOrKoopaJrActorInSphere(const al::LiveActor* pActor,
                                                                    f32 radius);
    al::LiveActor* tryFindNearestActivePlayerActorInCylinder(const al::LiveActor* pActor, f32 radius,
                                                             f32 bottom, f32 top);
    al::LiveActor* tryFindNearestActivePlayerOrKoopaJrActorInCylinder(const al::LiveActor* pActor,
                                                                      f32 radius, f32 bottom,
                                                                      f32 top);
    al::LiveActor* findNearestActivePlayerActor(const al::LiveActor* pActor);
    al::LiveActor* findNearestActivePlayerOrKoopaJrActor(const al::LiveActor* pActor);
    al::LiveActor* findNearestAnglePlayerActorH(const al::LiveActor* pActor,
                                                const sead::Vector3f& rDir,
                                                const sead::Vector3f& rOrigin);
    al::LiveActor* findNearestAnglePlayerActorH(const al::LiveActor* pActor,
                                                const sead::Vector3f& rDir);
    al::LiveActor* tryFindNearestPlayerActorByUserId(const al::LiveActor* pActor, s32 userId,
                                                     f32 radius);
    al::LiveActor* findRandomPlayerActor(const al::LiveActor* pActor);
    s32 calcActivePlayerNum(const al::LiveActor* pActor);
    u32 calcPlayerListOrderByDistance(const al::LiveActor* pActor, const al::LiveActor** pPlayerList,
                                      u32 listSize);
    al::LiveActor* findPlayerActorFirstByCharacterType(const al::LiveActor* pActor,
                                                       s32 characterType);
    al::LiveActor* findPlayerActorFirstByUserId(const al::PlayerHolder* pHolder, s32 userId);
    al::LiveActor* tryFindActivePlayerActorFirstByUserId(const al::LiveActor* pActor, s32 userId);
    al::LiveActor* tryFindAlivePlayerActorFirstByUserId(const al::LiveActor* pActor, s32 userId);
    bool isOnlyActivePlayerWithUserId(const al::LiveActor* pActor);
    void pauseAllPlayerAmiiboDirector(al::PlayerHolder* pHolder, bool isPause);
    void endPauseAllPlayerAmiiboDirector(al::PlayerHolder* pHolder);
    void setPlayerColorAnimBySensor(al::LiveActor* pActor, const al::HitSensor* pSensor,
                                    const char* pAnimName);
    void setPlayerColorAnimByCharacterType(al::LiveActor* pActor, s32 characterType,
                                           const char* pAnimName);
    void setPlayerColorAnimByControlUserId(al::LiveActor* pActor, s32 userId,
                                           const char* pAnimName);
    void setPlayerColorAnimDefault(al::LiveActor* pActor, const char* pAnimName);
    void resetPlayerAirLimitedAction(al::HitSensor* pSensor);
    void createInvincibleUbo(al::LiveActor* pActor);
    void createInvincibleUboWithSubActor(al::LiveActor* pActor);
    void setInvincibleColor(al::LiveActor* pActor, const sead::Color4f& rColor);
    void setInvincibleColorWithSubActor(al::LiveActor* pActor, const sead::Color4f& rColor);
    const sead::Color4f& getPlayerInvincibleColor(const al::HitSensor* pSensor);
    bool isPlayerInvincibleModelAppear(const al::HitSensor* pSensor);
    void validatePlayerMash(const al::HitSensor* pSensor);
    void invalidatePlayerMash(const al::HitSensor* pSensor);
    bool isPlayerMash(const al::HitSensor* pSensor);
    bool isGiantPlayerWalking(const al::HitSensor* pSensor);
    bool isGiantPlayerRunning(const al::HitSensor* pSensor);
    bool isGiantPlayerLanding(const al::HitSensor* pSensor);
    void cancelPanelDash(const al::HitSensor* pSensor);
    void cancelPanelDash(al::LiveActor* pActor);
    void showPlayer(const al::HitSensor* pSensor);
    void showPlayer(const al::LiveActor* pActor);
    void hidePlayer(const al::HitSensor* pSensor);
    void hidePlayer(const al::LiveActor* pActor);
    void showPlayerFur(const al::HitSensor* pSensor);
    void hidePlayerFur(const al::HitSensor* pSensor);
    void validatePlayerDynamics(const al::HitSensor* pSensor);
    void invalidatePlayerDynamics(const al::HitSensor* pSensor);
    void resetPlayerDynamics(const al::HitSensor* pSensor);
    void resetPlayerDynamics(const al::LiveActor* pActor);
    void validatePlayerDamage(const al::HitSensor* pSensor);
    void validatePlayerDamage(const al::LiveActor* pActor);
    void updateMaterialRouteDokan(al::HitSensor* pSensor, bool isInRouteDokan);
    void updateMaterialGoalPole(al::HitSensor* pSensor, const char* pMaterialCode);
    void setAliveWatcherToAudio(PlayerAliveWatcher* pWatcher, const al::PlayerHolder* pHolder);
    void clearPlayerCollisionInfo(const al::HitSensor* pSensor);
    void clearPlayerExPush(const al::HitSensor* pSensor);
    s32 getPlayerCeilingCheckLevel(const al::HitSensor* pSensor);
    void offCalcAndDrawPlayerEffect(al::LiveActor* pActor);
    void onCalcAndDrawPlayerEffect(al::LiveActor* pActor);
    void validatePlayerEffect(al::LiveActor* pActor);
    void invalidatePlayerEffect(al::LiveActor* pActor);
    bool isValidPlayerEffect(al::LiveActor* pActor);
    void validatePlayerWaterEffect(al::LiveActor* pActor);
    void invalidatePlayerWaterEffect(al::LiveActor* pActor);
    bool isValidPlayerWaterEffect(al::LiveActor* pActor);
    void killAllPlayersEffect(al::LiveActor* pActor);
    void killAllPlayerEffect(al::LiveActor* pActor);
    f32 getPlayerShadowLength(const al::HitSensor* pSensor);
    void appearPlayerPrePassLight(const al::HitSensor* pSensor, const char* pName);
    void killPlayerPrePassLight(const al::HitSensor* pSensor, const char* pName);
    void setPlayerTrans(al::LiveActor* pActor, const sead::Vector3f& rTrans);
    void setPlayerFrontVec(al::LiveActor* pActor, const sead::Vector3f& rFront);
    void setPlayerUpVec(al::LiveActor* pActor, const sead::Vector3f& rUp);
    void setPlayerVelocity(al::LiveActor* pActor, const sead::Vector3f& rVelocity);
    void setPlayerUseOldParams(al::PlayerHolder* pHolder, bool isOld);
    void setPlayerUseInputForHold(al::PlayerHolder* pHolder);
    void setPlayerUseKeyConfigForHold(al::PlayerHolder* pHolder);
    void setRequestToggleInputForHold(al::LiveActor* pActor);
    bool isPlayerChangeAllowed(al::PlayerHolder* pHolder, s32 port);
    s32 getNextValidCharType(GameDataHolder* pHolder, al::LiveActor* pActor, bool isNext);
    PlayerActor* changeCharType(GameDataHolder* pHolder, al::LiveActor* pActor, s32 characterType);
    void tryRequestClearFlingPoleDashFlag(al::LiveActor* pActor);
    void tryRequestClearDashFlag(al::LiveActor* pActor);
    void setSilentLand(al::LiveActor* pActor);
    void cancelAllPlayersForDemo(al::LiveActor* pActor);
    void clearScannedAmiiboList(al::LiveActor* pActor);
    void setBossPlayReportTaskData(al::BossPlayReportTask* pTask, const GameDataHolder* pHolder);
    bool tryRegisterBossPlayReport(al::NetworkSystem* pNetworkSystem,
                                   const GameDataHolder* pHolder);
    const char* getFloorCodeName(s32 floorCode);
    s32 getFloor2Code(const al::Triangle& rTriangle);
    const char* getWallCodeName(s32 wallCode);
    s32 getWallCode(const al::Triangle& rTriangle);
    const char* getCameraCodeName(s32 cameraCode);
    s32 getCameraCode(const al::Triangle& rTriangle);
    const char* getMaterialCodeName(s32 materialCode);
    bool isCollidedDamageFire(const al::LiveActor* pActor);
    bool isCollidedPoison(const al::LiveActor* pActor);
    bool isCollidedInkSlow(const al::LiveActor* pActor);
    bool isCollidedNeedle(const al::LiveActor* pActor);
    const char* getEffectCodeName(const char* pCode, const char* pMaterialCode);
    bool sendMsgFlingPoleDash(al::HitSensor* pReceiver, al::HitSensor* pSender, s32 frame);
};
