#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
    class IUseEffectKeeper;

    void emitEffectCurrentPos(IUseEffectKeeper*, const char*);
    void tryUpdateEffectMaterialCode(IUseEffectKeeper* pKeeper, const char* pMaterialCode);
    void resetEffectMaterialCode(IUseEffectKeeper* pKeeper);
    void updateEffectMaterialWet(IUseEffectKeeper* pKeeper, bool isWet);
    void updateEffectMaterialWater(IUseEffectKeeper* pKeeper, bool isWater);
    void emitEffect(IUseEffectKeeper*, const char*, const sead::Vector3f*);
    bool isEffectExist(IUseEffectKeeper*, const char*);
    bool isEffectSnapshotCameraMode(IUseEffectKeeper*, const char*);
    bool tryEmitEffect(IUseEffectKeeper*, const char*, const sead::Vector3f*);
    void deleteEffect(IUseEffectKeeper*, const char*);
    bool tryDeleteEffect(IUseEffectKeeper*, const char*);
    bool tryDeleteEffectAndParticle(IUseEffectKeeper*, const char*);
    void deleteEffectAll(IUseEffectKeeper*);
    bool tryKillEmitterAndParticleAll(IUseEffectKeeper*);
    bool tryDeleteEmitterAndParticleAll(IUseEffectKeeper*);
    void onCalcAndDrawEffect(IUseEffectKeeper*);
    void offCalcAndDrawEffect(IUseEffectKeeper*);
    void updateEffects(IUseEffectKeeper*);
    void forceSetStopCalcAndDraw(IUseEffectKeeper*, bool);
    bool isEffectEmitting(const IUseEffectKeeper*, const char*);
    bool isEffectEmittingFully(const IUseEffectKeeper*, const char*);
    void setEffectEmitRatio(IUseEffectKeeper*, const char*, f32);
    void setEffectEmitterScale(IUseEffectKeeper*, const char*, const sead::Vector3f&);
    void setEffectScale(IUseEffectKeeper*, const char*, const sead::Vector3f&);
    void setEffectEmitterVolumeScale(IUseEffectKeeper*, const char*, const sead::Vector3f&);
    void setEffectParticleScale(IUseEffectKeeper*, const char*, f32);
    void setEffectFollowPosPtr(IUseEffectKeeper*, const char*, const sead::Vector3f*);
    void setEffectFollowMtxPtr(IUseEffectKeeper*, const char*, const sead::Matrix34f*);
    void setEffectNamedMtxPtr(IUseEffectKeeper*, const char*, const sead::Matrix34f*);
    bool trySetEffectNamedMtxPtr(IUseEffectKeeper*, const char*, const sead::Matrix34f*);
};  // namespace al