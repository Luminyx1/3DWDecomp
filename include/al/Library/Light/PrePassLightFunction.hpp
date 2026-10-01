#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class TextureSampler;
}

namespace al {
class LiveActor;
class PrePassLightBase;

bool isActivePrePassLight(const LiveActor* pActor, const char* pName);
bool isExistPrePassLight(const LiveActor* pActor, const char* pName);
PrePassLightBase* getPrePassLineLight(const LiveActor* pActor, const char* pName);
void appearPrePassLight(const LiveActor* pActor, const char* pName, s32 step);
void killPrePassLight(const LiveActor* pActor, const char* pName, s32 step);
void appearPrePassLightAll(const LiveActor* pActor, s32 step);
void killPrePassLightAll(const LiveActor* pActor, s32 step);
void setPrePassLightOffset(const LiveActor* pActor, const char* pName, const sead::Vector3f& rOffset);
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, const sead::Color4f& rColor);
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, f32 rate);
void requestPrePassLightColor(const LiveActor* pActor, const char* pName, const char* pColorName,
                              f32 rate);
const sead::Color4f& getPrePassUserColor(const LiveActor* pActor, const char* pName);
void setEnablePrePassLightSpecular(const LiveActor* pActor, const char* pName, bool isEnable);
f32 getPrePassPointLightRadius(const LiveActor* pActor, const char* pName);
void setPrePassPointLightRadius(const LiveActor* pActor, const char* pName, f32 radius);
void getPrePassSpotLightInfo(const LiveActor* pActor, const char* pName, f32* pDegree, f32* pLength,
                             sead::Vector3f* pDir, sead::Vector3f* pPos);
void setPrePassSpotLightLength(const LiveActor* pActor, const char* pName, f32 length);
f32 getPrePassSpotLightCurrentLength(const LiveActor* pActor, const char* pName);
void setPrePassSpotLightDegree(const LiveActor* pActor, const char* pName, f32 degree);
bool isPrePassSpotLightStrikeCollision(const LiveActor* pActor, const char* pName);
void getPrePassProjLightInfo(const LiveActor* pActor, const char* pName, f32* pFovyDegree,
                             f32* pLength, sead::Vector3f* pDir, sead::Vector3f* pPos);
void getPrePassProjOrthoLightInfo(const LiveActor* pActor, const char* pName, f32* pLength,
                                  sead::Vector3f* pSize, sead::Vector3f* pDir, sead::Vector3f* pPos);
void setPrePassProjLightFar(const LiveActor* pActor, const char* pName, f32 far);
void setPrePassProjLightFovyDegree(const LiveActor* pActor, const char* pName, f32 fovyDegree);
void setPrePassProjLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                               bool isSoft, f32 param);
void setPrePassProjOrthoLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                                    bool isSoft, f32 param);
void setPrePassSpotLightShadow(const LiveActor* pActor, const char* pName, bool isEnable,
                               bool isSoft, f32 param);
void initPrePassLightMtxConnector(const LiveActor* pActor, const char* pName,
                                  const sead::Matrix34f* pMtx);
const agl::TextureSampler* getTexIrradianceObj(const LiveActor* pActor);
}  // namespace al
