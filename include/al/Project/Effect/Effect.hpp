#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Model/JointMtxPtr.hpp"

namespace al {
class EffectCameraHolder;
class EffectEmitter;
struct EffectInfo;
class EffectSystemInfo;

class Effect {
public:
    Effect(const EffectSystemInfo* pSystemInfo, const EffectInfo* pInfo,
           const sead::Vector3f* pTrans, const sead::Vector3f* pRotate,
           const sead::Matrix34f* pMtx, u64 userData);

    void setCameraHolder(EffectCameraHolder* pCameraHolder);
    void setPosPtr(const sead::Vector3f* pPos);
    void setMtxPtr(const sead::Matrix34f* pMtx);
    bool update();
    void setFarClip(bool isFarClip);
    bool tryUpdateMaterial(const char* pMaterialCode, const bool (&rPrefixFlags)[4]);
    void emitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos);
    void emitEmitters(const sead::Vector3f* pPos, bool isCurrentPos);
    bool tryEmitEmitters(const sead::Vector3f* pPos, bool isCurrentPos);
    bool tryEmitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos);
    void tryDeleteEmitters();
    void deleteAndClearEmitter();
    bool isOneTimeFade() const;
    void tryKillEmitterAndParticleAll();
    void setStopCalcAndDraw_CAFE(bool isStop);
    void setStopCalcAndDraw(bool isStop);
    void forceSetStopCalcAndDraw(bool isStop);
    void setActorClip(bool isClip);
    void setEnableDraw(bool isEnable);
    void setEmitRatio(f32 ratio);
    void setEmitterScale(const sead::Vector3f& rScale);
    void setEmitterAllScale(const sead::Vector3f& rScale);
    void setEmitterVolumeScale(const sead::Vector3f& rScale);
    void setParticleScale(f32 scale);
    void setParticleScale(const sead::Vector3f& rScale);
    void setParticleAlpha(f32 alpha);
    void setParticleColor(const sead::Color4f& rColor);
    void setParticleLifeScale(f32 scale);
    void setEmitterColors(const sead::Color4f& rColor0, const sead::Color4f& rColor1);
    const sead::Matrix34f* getViewMtxPtr() const;
    bool isLoopOrInfinity() const;
    bool isEmitterActive() const;
    bool isEmitterActiveFully() const;

    const char* getName() const { return mName; }
    s32 getEmitterNum() const { return mEmitterNum; }
    EffectEmitter* getEmitter(s32 index) const { return mEmitters[index]; }
    const EffectInfo* getEffectInfo() const { return mEffectInfo; }
    const sead::Vector3f* getPosPtr() const { return mPosPtr; }

    const char* mName;
    EffectEmitter** mEmitters;
    s32 mEmitterNum;
    u8 _14[0xc];
    const EffectInfo* mEffectInfo;
    const sead::Vector3f* mPosPtr;
    u8 _30[0x8];
    JointMtxPtr mJointMtxPtr;
};
}  // namespace al
