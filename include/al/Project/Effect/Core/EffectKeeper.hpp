#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include <basis/seadTypes.h>

#include "Project/Effect/Effect.hpp"

namespace agl {
class DrawContext;
class TextureData;
}  // namespace agl

namespace agl::sdw {
class DepthShadow;
}  // namespace agl::sdw

namespace sead {
class Viewport;
}  // namespace sead

namespace al {
class Effect;
struct EffectResourceInfo;
class EmitterSetResourceInfo;
class EffectSystem;
class EffectSystemInfo;
struct EffectUserInfo;
class IUseCamera;
class IUseEffectKeeper;
class IUseLayout;
class ModelKeeper;
class MtxPtrHolder;

class EffectKeeper {
public:
    EffectKeeper(const EffectSystemInfo* pSystemInfo, const char* pName,
                 const sead::Vector3f* pTrans, const sead::Vector3f* pScale,
                 const sead::Matrix34f* pMtx);

    void update();
    void tryUpdateMaterial(const char* pMaterialCode);
    void updatePrefix(const EffectPrefixType& rType, bool isOn);
    void emitEffectCurrentPos(const char* pName);
    Effect* findEffect(const char* pName) const;
    void emitEffect(const char* pName, const sead::Vector3f* pPos);
    bool tryEmitEffect(const char* pName, const sead::Vector3f* pPos);
    void deleteEffect(const char* pName);
    void tryDeleteEffect(const char* pName);
    void tryDeleteEffectAndParticle(const char* pName);
    void tryKillEmitterAndParticleAll();
    void deleteEffectAll();
    void deleteAndClearEffectAll();
    void onCalcAndDraw();
    void offCalcAndDraw();
    void forceSetStopCalcAndDraw(bool isStop);
    void setEnableDraw(bool isEnable);
    void setEnableDraw(bool isEnable, const char* pName);
    void setEmitRatio(const char* pName, f32 ratio);
    void setEmitterScale(const char* pName, const sead::Vector3f& rScale);
    void setEmitterAllScale(const char* pName, const sead::Vector3f& rScale);
    void setEmitterVolumeScale(const char* pName, const sead::Vector3f& rScale);
    void setParticleScale(const char* pName, f32 scale);
    void setParticleScale(const char* pName, const sead::Vector3f& rScale);
    void setParticleAlpha(const char* pName, f32 alpha);
    void setParticleColor(const char* pName, const sead::Color4f& rColor);
    void setParticleLifeScale(const char* pName, f32 scale);
    void setEmitterColors(const char* pName, const sead::Color4f& rColor0,
                          const sead::Color4f& rColor1);
    const sead::Matrix34f* findMtxPtr(const char* pName);
    Effect* tryFindEffect(const char* pName) const;

    s32 getEffectNum() const { return mEffectNum; }
    Effect* getEffect(s32 index) const { return mEffects[index]; }
    MtxPtrHolder* getMtxPtrHolder() const { return mMtxPtrHolder; }

    const char* mName;
    s32 mEffectNum = 0;
    Effect** mEffects = nullptr;
    const char* mMaterialCode = "";
    bool mIsEmitted = false;
    bool mIsExistFarClipEffect = false;
    bool mPrefixFlags[4] = {};
    EffectUserInfo* mEffectUserInfo = nullptr;
    MtxPtrHolder* mMtxPtrHolder = nullptr;
};

static_assert(sizeof(EffectKeeper) == 0x38);
}  // namespace al

namespace alEffectKeeperInitFunction {
void setupModelToEffectKeeper(al::EffectKeeper* pEffectKeeper, const al::ModelKeeper* pModelKeeper);
void setupLayoutToEffectKeeper(al::EffectKeeper* pEffectKeeper, const al::IUseLayout* pLayout);
void setupCameraToEffectKeeper(al::EffectKeeper* pEffectKeeper, const al::IUseCamera* pCamera);
void updateNamedMtxPtr(al::EffectKeeper* pEffectKeeper, const char* pName);
}  // namespace alEffectKeeperInitFunction

namespace alEffectFunction {
al::EffectUserInfo* tryFindEffectUser(const al::EffectSystemInfo* pSystemInfo,
                                      const char* pName);
void initResourceInfo(const al::EffectSystemInfo* pSystemInfo,
                      al::EffectResourceInfo* pResourceInfo);
al::EmitterSetResourceInfo* tryFindEffectResouceInfo(const al::EffectSystemInfo* pSystemInfo,
                                                     const char* pName);
bool emitEffectIfExist(al::IUseEffectKeeper* pUser, const char* pName, const sead::Vector3f* pPos);
}  // namespace alEffectFunction
