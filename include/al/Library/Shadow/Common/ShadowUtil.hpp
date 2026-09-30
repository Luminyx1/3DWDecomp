#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class ActorInitInfo;
    class LiveActor;
    class ShadowMaskBase;
    class ShadowMaskDrawCategory;

    bool isExistShadow(LiveActor*);
    bool isExistShadow(LiveActor*, const char*);
    bool isHideShadow(const LiveActor*);
    void hideShadow(LiveActor*);
    void showShadow(LiveActor*);
    void hideShadowDepth(LiveActor*);
    void showShadowDepth(LiveActor*);

    void showShadow(LiveActor* pActor, ShadowMaskBase* pMask);
    void showShadow(LiveActor* pActor, const char* pName);
    void showShadow(LiveActor* pActor, ShadowMaskDrawCategory category);
    void hideShadow(LiveActor* pActor, ShadowMaskBase* pMask);
    void hideShadow(LiveActor* pActor, const char* pName);
    void hideShadow(LiveActor* pActor, ShadowMaskDrawCategory category);
    bool isHideShadow(LiveActor* pActor, const char* pName);
    void validateShadow(LiveActor* pActor, ShadowMaskBase* pMask);
    void invalidateShadow(LiveActor* pActor, ShadowMaskBase* pMask);
    void validateShadow(LiveActor* pActor, const char* pName);
    void invalidateShadow(LiveActor* pActor, const char* pName);
    void invalidateShadow(LiveActor* pActor, ShadowMaskDrawCategory category);
    void invalidateShadowIntensityAll(LiveActor* pActor);

    void setShadowFixed(LiveActor*, bool);
    void setShadowDropDir(LiveActor* pActor, const sead::Vector3f& rDir);
    void setShadowDropDir(LiveActor* pActor, const sead::Vector3f& rDir, const char* pName);
    void setShadowDropDirActorDown(LiveActor* pActor);
    void setShadowMaskSize(LiveActor* pActor, const char* pName, const sead::Vector3f& rSize);
    void setShadowMaskSize(LiveActor* pActor, const char* pName, f32 x, f32 y, f32 z);
    void calcShadowMaskSize(sead::Vector3f* pOut, LiveActor* pActor, const char* pName);
    f32 getShadowDropLength(const LiveActor* pActor, const char* pName);
    void setShadowDropLength(LiveActor* pActor, f32 length);
    void setShadowDropLength(LiveActor*, f32, const char*);
    void setShadowDropLengthScaleWithDrawCategory(LiveActor* pActor, f32 scale,
                                                  ShadowMaskDrawCategory category);
    void setShadowDropLengthWithDrawCategory(LiveActor* pActor, f32 length,
                                             ShadowMaskDrawCategory category);
    void setShadowDropLengthEvenWithTarget(ShadowMaskBase* pMask, const char* pTargetName,
                                           const sead::Vector3f& rPos);
    void setShadowDropLengthEvenWithTarget(ShadowMaskBase* pMask, const ShadowMaskBase* pTarget,
                                           const sead::Vector3f& rPos);
    void setShadowDropLengthEvenWithDrawCategory(LiveActor* pActor, ShadowMaskDrawCategory category,
                                                 const LiveActor* pTargetActor,
                                                 const char* pTargetName);
    void setShadowDropLengthEvenPlaneNormal(const LiveActor* pActor, const sead::Vector3f& rNormal);
    f32 getShadowDropLengthMax(const LiveActor* pActor);

    void setShadowIntensityUser(LiveActor*, u8, const char*);
    f32 getShadowIntensity(LiveActor* pActor, const char* pName);
    bool isShadowMaskDrawCategoryAO(const ShadowMaskBase* pMask);
    bool isShadowMaskDrawCategoryLightScale(const ShadowMaskBase* pMask);
    f32 getShadowTextureFixedScale(const LiveActor* pActor, const char* pName);
    void setShadowTextureFixedScale(const LiveActor* pActor, const char* pName, f32 scale);
    const sead::Vector3f& getShadowMaskOffset(const LiveActor* pActor, const char* pName);
    void setShadowMaskOffset(const LiveActor* pActor, const sead::Vector3f& rOffset,
                             const char* pName);
    bool trySetShadowLength(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName);

};  // namespace al
