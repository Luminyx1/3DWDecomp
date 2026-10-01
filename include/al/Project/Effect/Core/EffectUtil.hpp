#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class IUseEffectKeeper;

void emitEffectCurrentPos(IUseEffectKeeper* pUser, const char* pName);
void emitEffect(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos);
bool isEffectExist(IUseEffectKeeper* pUser, const char* pName);
bool isEffectSnapshotCameraMode(IUseEffectKeeper* pUser, const char* pName);
bool tryEmitEffect(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos);
void deleteEffect(IUseEffectKeeper* pUser, const char* pName);
void tryDeleteEffect(IUseEffectKeeper* pUser, const char* pName);
void tryDeleteEffectAndParticle(IUseEffectKeeper* pUser, const char* pName);
void deleteEffectAll(IUseEffectKeeper* pUser);
void tryKillEmitterAndParticleAll(IUseEffectKeeper* pUser);
void tryDeleteEmitterAndParticleAll(IUseEffectKeeper* pUser);
void onCalcAndDrawEffect(IUseEffectKeeper* pUser);
void offCalcAndDrawEffect(IUseEffectKeeper* pUser);
void updateEffects(IUseEffectKeeper* pUser);
void forceSetStopCalcAndDraw(IUseEffectKeeper* pUser, bool isStop);
bool isEffectEmitting(const IUseEffectKeeper* pUser, const char* pName);
bool isEffectEmittingFully(const IUseEffectKeeper* pUser, const char* pName);
void setEffectEmitRatio(IUseEffectKeeper* pUser, const char* pName, f32 ratio);
void setEffectEmitterScale(IUseEffectKeeper* pUser, const char* pName,
                           const sead::Vector3f& rScale);
void setEffectAllScale(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f& rScale);
void setEffectEmitterVolumeScale(IUseEffectKeeper* pUser, const char* pName,
                                 const sead::Vector3f& rScale);
void setEffectParticleScale(IUseEffectKeeper* pUser, const char* pName, f32 scale);
void setEffectParticleScale(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Vector3f& rScale);
void setEffectParticleAlpha(IUseEffectKeeper* pUser, const char* pName, f32 alpha);
void setEffectParticleColor(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Color4f& rColor);
void setEffectEmitterColors(IUseEffectKeeper* pUser, const char* pName,
                            const sead::Color4f& rColor0, const sead::Color4f& rColor1);
void setParticleLifeScale(IUseEffectKeeper* pUser, const char* pName, f32 scale);
void setEffectParticleDirectionalVel(IUseEffectKeeper* pUser, const char* pName, f32 vel);
void setEffectFollowPosPtr(IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos);
void setEffectFollowMtxPtr(IUseEffectKeeper* pUser, const char* pName,
                           const sead::Matrix34f* pMtx);
void setEffectNamedMtxPtr(IUseEffectKeeper* pUser, const char* pName, const sead::Matrix34f* pMtx);
void trySetEffectNamedMtxPtr(IUseEffectKeeper* pUser, const char* pName,
                             const sead::Matrix34f* pMtx);
void tryUpdateEffectMaterialCode(IUseEffectKeeper* pUser, const char* pMaterialCode);
void resetEffectMaterialCode(IUseEffectKeeper* pUser);
void updateEffectMaterialWater(IUseEffectKeeper* pUser, bool isOn);
void updateEffectMaterialRouteDokan(IUseEffectKeeper* pUser, bool isOn);
void updateEffectMaterialWet(IUseEffectKeeper* pUser, bool isOn);
void updateEffectMaterialPuddle(IUseEffectKeeper* pUser, bool isOn);
const sead::Vector3f* getEffectPosPtr(IUseEffectKeeper* pUser, const char* pName);
}  // namespace al
