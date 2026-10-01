#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>

#include "Library/Model/JointMtxPtr.hpp"

namespace al {
class EffectCameraHolder;
class EffectEmitter;
struct EffectInfo;
class EffectSystem;
class EffectSystemInfo;

SEAD_ENUM(EffectPrefixType, Water, RouteDokan, Wet, Shallow)

class Effect {
public:
    Effect(const EffectSystemInfo* pSystemInfo, const EffectInfo* pInfo, const sead::Vector3f* pTrans,
           const sead::Vector3f* pScale, const sead::Matrix34f* pMtx, u64 userData);

    void setCameraHolder(EffectCameraHolder* pCameraHolder);
    void setPosPtr(const sead::Vector3f* pPos);
    void setMtxPtr(const sead::Matrix34f* pMtx);
    bool update();
    void setFarClip(bool isFarClip);
    void tryUpdateMaterial(const char* pMaterialCode, const bool (&rIsPrefix)[4]);
    void emitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos);
    bool emitEmitters(const sead::Vector3f* pPos, bool isCurrentMaterial);
    bool tryEmitEmitters(const sead::Vector3f* pPos, bool isCurrentMaterial);
    bool tryEmitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos);
    bool tryDeleteEmitters();
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

    void initMtxPtr(JointMtxPtr mtxPtr) { mJointMtxPtr = mtxPtr; }

private:
    const char* mName;
    EffectEmitter** mEmitters;
    s32 mEmitterNum;
    EffectSystem* mEffectSystem;
    const EffectInfo* mEffectInfo;
    const sead::Vector3f* mPosPtr;
    const sead::Vector3f* mScalePtr;
    JointMtxPtr mJointMtxPtr;
    const sead::Matrix34f* mViewMtxPtr;
    EffectCameraHolder* mCameraHolder;
    bool mIsEmitted;
    bool mIsActorClip;
    bool mIsFarClip;
    sead::FixedSafeString<64> mMaterialName;
    u64 _b8;
    u64 mUserData;
};

static_assert(sizeof(Effect) == 0xc8);
}  // namespace al
