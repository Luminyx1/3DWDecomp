#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class HitSensor;
class LayoutActor;
} // namespace al

class IUsePlayerPuppet;
class PlayerBindEndParam;

namespace rc {
const char16_t* getPlayerCharacterMessageName(const al::LayoutActor* pActor, s32 characterType);
IUsePlayerPuppet* startPuppet(al::HitSensor* pBinderSensor, al::HitSensor* pPlayerSensor);
void endBindAndPuppetNull(IUsePlayerPuppet** ppPuppet, const PlayerBindEndParam* pParam);
void endBindOnGroundAndPuppetNull(IUsePlayerPuppet** ppPuppet);
void endBindSquatAndPuppetNull(IUsePlayerPuppet** ppPuppet);
void endBindForceAbyssAndPuppetNull(IUsePlayerPuppet** ppPuppet);
void setPuppetTrans(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rTrans);
void setPuppetVelocity(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rVelocity);
void setPuppetFrontVec(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rFront);
void setPuppetUpVec(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rUp);
void setPuppetQuat(IUsePlayerPuppet* pPuppet, const sead::Quatf& rQuat);
void setPuppetMtx(IUsePlayerPuppet* pPuppet, const sead::Matrix34f* pMtx);
const sead::Vector3f& getPuppetTrans(const IUsePlayerPuppet* pPuppet);
const sead::Vector3f& getPuppetVelocity(const IUsePlayerPuppet* pPuppet);
const sead::Vector3f& getPuppetFrontVec(const IUsePlayerPuppet* pPuppet);
const sead::Vector3f& getPuppetUpVec(const IUsePlayerPuppet* pPuppet);
void startPuppetAction(IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName);
void setPuppetActionRate(IUsePlayerPuppet* pPuppet, f32 rate);
bool isPuppetActionEnd(const IUsePlayerPuppet* pPuppet);
bool isPuppetAction(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName);
void setPuppetActionFrame(IUsePlayerPuppet* pPuppet, f32 frame);
f32 getPuppetActionFrame(const IUsePlayerPuppet* pPuppet);
f32 getPuppetActionFrameMax(const IUsePlayerPuppet* pPuppet);
f32 getPuppetActionFrameMax(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName);
void setPuppetBlendAnimWeight(IUsePlayerPuppet* pPuppet, f32 weight0, f32 weight1, f32 weight2,
                              f32 weight3, f32 weight4, f32 weight5);
f32 getPuppetBlendAnimWeight(const IUsePlayerPuppet* pPuppet, u32 index);
bool isPuppetStickOn(const IUsePlayerPuppet* pPuppet);
s32 getPuppetInputPort(const IUsePlayerPuppet* pPuppet);
f32 getPuppetStickX(const IUsePlayerPuppet* pPuppet);
f32 getPuppetStickY(const IUsePlayerPuppet* pPuppet);
const sead::Vector3f& getPuppetStickWorldWithSnap(const IUsePlayerPuppet* pPuppet);
const sead::Vector3f& getPuppetStickWorldWithoutSnap(const IUsePlayerPuppet* pPuppet);
bool isPuppetTrigJumpButton(const IUsePlayerPuppet* pPuppet);
bool isPuppetTrigJumpButtonWithoutPrecedeInput(const IUsePlayerPuppet* pPuppet);
bool isPuppetHoldJumpButton(const IUsePlayerPuppet* pPuppet);
bool isPuppetHoldSquatButton(const IUsePlayerPuppet* pPuppet);
bool isPuppetTrigSquatButton(const IUsePlayerPuppet* pPuppet);
bool isPuppetTrigDashButton(const IUsePlayerPuppet* pPuppet);
bool isPuppetHoldDashButton(const IUsePlayerPuppet* pPuppet);
s32 getPuppetTrigJumpFrame(const IUsePlayerPuppet* pPuppet);
void calcPuppetQuat(sead::Quatf* pQuat, const IUsePlayerPuppet* pPuppet);
void calcPuppetQuatAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans,
                            const IUsePlayerPuppet* pPuppet);
void startPuppetSe(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rSeName);
void changePuppetMaterialCode(IUsePlayerPuppet* pPuppet, const sead::SafeString& rMaterialCode);
void damagePuppet(IUsePlayerPuppet* pPuppet);
void tryDamagePuppet(IUsePlayerPuppet* pPuppet);
void checkDeathMapCodePuppet(IUsePlayerPuppet* pPuppet);
void hidePuppetAllParts(IUsePlayerPuppet* pPuppet);
void hidePuppet(IUsePlayerPuppet* pPuppet);
void hidePuppetFur(IUsePlayerPuppet* pPuppet);
void hidePuppetShadow(IUsePlayerPuppet* pPuppet);
void hidePuppetSilhouette(IUsePlayerPuppet* pPuppet);
void hidePuppetRain(IUsePlayerPuppet* pPuppet);
void showPuppetAllParts(IUsePlayerPuppet* pPuppet);
void showPuppet(IUsePlayerPuppet* pPuppet);
void showPuppetFur(IUsePlayerPuppet* pPuppet);
void showPuppetShadow(IUsePlayerPuppet* pPuppet);
void showPuppetSilhouette(IUsePlayerPuppet* pPuppet);
bool isPuppetHidden(IUsePlayerPuppet* pPuppet);
void moveSimplePuppet(IUsePlayerPuppet* pPuppet);
void solveAirPuppet(IUsePlayerPuppet* pPuppet);
void solveAirNoFloorPuppet(IUsePlayerPuppet* pPuppet);
void snapGroundPuppet(IUsePlayerPuppet* pPuppet);
void snapWallPuppet(IUsePlayerPuppet* pPuppet, bool isSnapAll);
bool isOnFloorPuppet(IUsePlayerPuppet* pPuppet);
bool isOnCeilingPuppet(IUsePlayerPuppet* pPuppet);
bool isOnWallPuppet(IUsePlayerPuppet* pPuppet);
bool isCollidedPuppet(IUsePlayerPuppet* pPuppet);
void getCeilingNormalPuppet(sead::Vector3f* pNormal, IUsePlayerPuppet* pPuppet);
void getWallNormalPuppet(sead::Vector3f* pNormal, IUsePlayerPuppet* pPuppet);
void clearPuppetCollisionInfo(IUsePlayerPuppet* pPuppet);
void clearPuppetExPush(IUsePlayerPuppet* pPuppet);
al::HitSensor* getPuppetSensor(IUsePlayerPuppet* pPuppet);
bool isPuppetSensor(IUsePlayerPuppet* pPuppet, const al::HitSensor* pSensor);
void validateSubActionPuppet(IUsePlayerPuppet* pPuppet);
void setPuppetAlphaCtrl(IUsePlayerPuppet* pPuppet, bool isEnable);
void invalidateSubActionPuppet(IUsePlayerPuppet* pPuppet);
void forceEndSubActionPuppet(IUsePlayerPuppet* pPuppet);
void invalidateUpperSubActionPuppet(IUsePlayerPuppet* pPuppet);
void invalidatePuppetInput(IUsePlayerPuppet* pPuppet, u32 frame);
void setPuppetGlideInhibitFrame(IUsePlayerPuppet* pPuppet, s32 frame);
void validatePuppetGetItem(IUsePlayerPuppet* pPuppet);
void invalidatePuppetGetItem(IUsePlayerPuppet* pPuppet);
void validatePuppetSensors(IUsePlayerPuppet* pPuppet);
void invalidatePuppetSensors(IUsePlayerPuppet* pPuppet);
void resetAirLimitedActionPuppet(IUsePlayerPuppet* pPuppet);
void validateMaterialRouteDokan(IUsePlayerPuppet* pPuppet);
void invalidateMaterialRouteDokan(IUsePlayerPuppet* pPuppet);
void tryDeleteEmitterAndParticleAll(IUsePlayerPuppet* pPuppet);
bool sendPuppetMsgForceAbyss(IUsePlayerPuppet* pPuppet, al::HitSensor* pSender);
bool sendPuppetMsgForceDash(IUsePlayerPuppet* pPuppet, al::HitSensor* pSender, s32 frame);
void validatePuppetDynamics(IUsePlayerPuppet* pPuppet);
void invalidatePuppetDynamics(IUsePlayerPuppet* pPuppet);
void resetPuppetDynamics(IUsePlayerPuppet* pPuppet);
void validatePuppetDamage(IUsePlayerPuppet* pPuppet);
void invalidatePuppetDamage(IUsePlayerPuppet* pPuppet, u32 frame);
void validatePuppetFlash(IUsePlayerPuppet* pPuppet);
void invalidatePuppetFlash(IUsePlayerPuppet* pPuppet);
void appearPuppetPrePassLight(IUsePlayerPuppet* pPuppet, const char* pName);
void killPuppetPrePassLight(IUsePlayerPuppet* pPuppet, const char* pName);
bool faceToDirection(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rDir, f32 maxDegree,
                     f32 endDegree);
} // namespace rc
