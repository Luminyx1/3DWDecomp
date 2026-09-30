#pragma once

#include <gfx/seadCamera.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace al {
class ByamlIter;
class ICameraInput;
class IUseAreaObj;
class IUseCollision;

struct SnapShotCameraSceneInfo {
    IUseAudioKeeper* audioKeeper;
    bool isValidLimitAtY;
    f32 limitAtY;
};

struct SnapShotCameraParam {
    bool hasMinFovyDegree = false;
    bool hasMaxFovyDegree = false;
    f32 minFovyDegree = 40.0f;
    f32 maxFovyDegree = 85.0f;
};

class SnapShotCameraCtrl : public NerveExecutor, public IUseAudioKeeper {
public:
    SnapShotCameraCtrl(const IUseAreaObj* pAreaObj, const SnapShotCameraSceneInfo* pSceneInfo,
                       bool isLongRange);

    void start(f32 fovyDegree);
    void load(const ByamlIter& rIter);
    void startReset(s32 step);
    void update(const sead::LookAtCamera& rCamera, const IUseCollision* pCollision,
                const ICameraInput* pInput);
    void makeLookAtCameraPost(sead::LookAtCamera* pCamera);
    void makeLookAtCameraLast(sead::LookAtCamera* pCamera) const;
    f32 getRotationScaler() const;
    void exeWait();
    void exeReset();

    AudioKeeper* getAudioKeeper() const override;

    f32 getFovyDegree() const { return mFovyDegree; }

    void setIsValidLookAtOffset(bool isValid) { mIsValidLookAtOffset = isValid; }

    void setIsValidZoomFovy(bool isValid) { mIsValidZoomFovy = isValid; }

    void setIsValidRoll(bool isValid) { mIsValidRoll = isValid; }

    void setIsEnableRoll(bool isEnable) { mIsEnableRoll = isEnable; }

    void setIsZoomAutoReset(bool isValid) { mIsZoomAutoReset = isValid; }

    void setIsValidMove(bool isValid) { mIsValidMove = isValid; }

    void setMaxZoomOutFovyDegree(f32 fovy) { mMaxZoomOutFovyDegree = fovy; }

    f32 getRollDegree() const { return mRollDegree; }

    const sead::Vector3f& getLookAtOffset() const { return mLookAtOffset; }

private:
    const IUseAreaObj* mAreaObj;
    const SnapShotCameraSceneInfo* mSceneInfo;
    SnapShotCameraParam* mParam = nullptr;
    const IUseCollision* mCollision = nullptr;
    bool mIsValidLookAtOffset = false;
    sead::Vector3f mLookAtOffset = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLookAtOffsetTarget = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mSafeLookAtOffset;
    sead::Vector3f mSafeLookAtOffsetTarget;
    bool mIsValidZoomFovy = false;
    f32 mFovyDegree = 0.0f;
    f32 mDefaultFovyDegree = 0.0f;
    f32 mFovyDegreeTarget = 0.0f;
    f32 mMaxZoomOutFovyDegree = -1.0f;
    bool mIsValidRoll = false;
    f32 mRollDegree = 0.0f;
    f32 mRollTarget = 0.0f;
    s32 mResetStep = -1;
    bool mIsZoomAutoReset = false;
    bool mIsEnableRoll = true;
    bool mIsValidMove = false;
    bool mIsInInk = false;
    bool mIsLongRange;
};

}  // namespace al
