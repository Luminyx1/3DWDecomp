#pragma once

#include <basis/seadTypes.h>
#include <cstring>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

namespace al {
class ByamlIter;

class SeMaterialSettingInfo {
public:
    static SeMaterialSettingInfo* createInfo(const ByamlIter& rIter) {
        SeMaterialSettingInfo* info = new SeMaterialSettingInfo;
        rIter.tryGetStringByKey(&info->mName, "Name");
        if (!rIter.tryGetStringByKey(&info->mResourceName, "ResourceName")) {
            return nullptr;
        }

        u32 soundId = alSoundNameUtil::getSoundId(info->mResourceName, false);
        info->mSoundId = soundId;
        if (AudioConst::SOUND_ID_INVALID == soundId) {
            return nullptr;
        }

        return info;
    }

    static s32 compareInfo(const SeMaterialSettingInfo* pA, const SeMaterialSettingInfo* pB) {
        return strcmp(pA->mName, pB->mName);
    }

    const char* mName = nullptr;
    const char* mResourceName = nullptr;
    u32 mSoundId = 0;
};

static_assert(sizeof(SeMaterialSettingInfo) == 0x18);

class SeResourceSpecificInfo {
public:
    static SeResourceSpecificInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeResourceSpecificInfo* pA, const SeResourceSpecificInfo* pB);

    const char* mName = nullptr;
    u32 mSoundId = AudioConst::SOUND_ID_INVALID;
    s32 _c = 0;
    s32 mLimitPlayingNum = 0;
    s32 mLimitTriggerFrame = 0;
    s32 mLimitTriggerNum = 0;
    s32 mDelayFrame = 0;
    s32 mDelayMaxNum = 0;
    f32 mDelayVolume = -1.0f;
    f32 mVolumeAfterGoal = -1.0f;
    bool mIsValidMatCodeLpf = false;
    bool mIsCmNg = false;
    bool mIsIgnoreDistPause = false;
    bool mIsIgnoreInTitleScene = false;
    AudioInfoList<SeMaterialSettingInfo>* mMaterialInfoList = nullptr;
    s32 mPlayerId = 0;
};

static_assert(sizeof(SeResourceSpecificInfo) == 0x40);

class InOutParam;

class SeResourceInfo {
public:
    static SeResourceInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeResourceInfo* pA, const SeResourceInfo* pB);

    const char* mName = nullptr;
    u32 mSoundId = AudioConst::SOUND_ID_INVALID;
    u32 mInputFunctionId = 0;
    InOutParam* mPitch = nullptr;
    InOutParam* mVolume = nullptr;
    InOutParam* mTempo = nullptr;
    const char* mEmitterName = nullptr;
    bool mIsSetParamMin = false;
    f32 mParamMin = 0.0f;
    s32 mLocalVarNo = -1;
    f32 mLfeSend = 0.0f;
    const SeResourceSpecificInfo* mSpecificInfo = nullptr;
};

static_assert(sizeof(SeResourceInfo) == 0x48);

class SePlayInfo {
public:
    static const s32 USE_DEFAULT_FADE_OUT_FRAME_NUM;

    static SePlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SePlayInfo* pA, const SePlayInfo* pB);

    const char* mName = nullptr;
    bool mIsLoop = false;
    s32 mFadeOutFrameNum = USE_DEFAULT_FADE_OUT_FRAME_NUM;
    AudioInfoList<SeResourceInfo>* mResourceInfoList = nullptr;
    const char* mRequestKeeperName = nullptr;
};

static_assert(sizeof(SePlayInfo) == 0x20);

class SePlayInfoInAction {
public:
    SePlayInfoInAction();
    SePlayInfoInAction(const SePlayInfoInAction& rOther);
    SePlayInfoInAction& operator=(const SePlayInfoInAction& rOther);

    static SePlayInfoInAction* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SePlayInfoInAction* pA, const SePlayInfoInAction* pB);

    const char* mName = nullptr;
    f32 mStartFrame = 0.0f;
    f32 mEndFrame = 0.0f;
    bool mIsOneTime = false;
};

static_assert(sizeof(SePlayInfoInAction) == 0x18);

class SeActionInfo {
public:
    SeActionInfo();
    SeActionInfo(const SeActionInfo& rOther);
    SeActionInfo& operator=(const SeActionInfo& rOther);

    static SeActionInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeActionInfo* pA, const SeActionInfo* pB);

    const char* mName;
    bool mIsStopPlayingSe;
    AudioInfoList<SePlayInfoInAction>* mPlayInfoList;
};

static_assert(sizeof(SeActionInfo) == 0x18);

class SeSoundSourceInfo {
public:
    SeSoundSourceInfo(const char* pName) : mName(pName) {}

    virtual void dummy();

    const char* mName;
};

class SeSoundSourceInfoAmbient : public SeSoundSourceInfo {
public:
    SeSoundSourceInfoAmbient(const char* pName) : SeSoundSourceInfo(pName) {}
};

class SeSoundSourceInfo3DPoint : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DPoint(const char* pName) : SeSoundSourceInfo(pName) {}
};

class SeSoundSourceInfo3DSphere : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DSphere(const char* pName) : SeSoundSourceInfo(pName) {}
    SeSoundSourceInfo3DSphere(const SeSoundSourceInfo3DSphere& rOther)
        : SeSoundSourceInfo(rOther.mName), mRadius(rOther.mRadius) {}

    f32 mRadius = 0.0f;
};

class SeSoundSourceInfo3DVector : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DVector(const char* pName) : SeSoundSourceInfo(pName), mVector(0.0f, 0.0f, 0.0f) {}
    SeSoundSourceInfo3DVector(const SeSoundSourceInfo3DVector& rOther) : SeSoundSourceInfo(rOther.mName) {
        mVector.x = rOther.mVector.x;
        mVector.y = rOther.mVector.y;
        mVector.z = rOther.mVector.z;
    }

    sead::Vector3f mVector;
};

class SeSoundSourceInfo3DBox : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DBox(const char* pName) : SeSoundSourceInfo(pName) {}
    SeSoundSourceInfo3DBox(const SeSoundSourceInfo3DBox& rOther)
        : SeSoundSourceInfo(rOther.mName), mMinX(rOther.mMinX), mMinY(rOther.mMinY), mMaxX(rOther.mMaxX),
          mMaxY(rOther.mMaxY) {}

    f32 mMinX = 0.0f;
    f32 mMinY = 0.0f;
    f32 mMaxX = 0.0f;
    f32 mMaxY = 0.0f;
};

class SeSoundSourceInfo3DRing : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DRing(const char* pName) : SeSoundSourceInfo(pName) {}
    SeSoundSourceInfo3DRing(const SeSoundSourceInfo3DRing& rOther)
        : SeSoundSourceInfo(rOther.mName), mRadius(rOther.mRadius) {}

    f32 mRadius = 0.0f;
};

class SeSoundSourceInfo3DCircle : public SeSoundSourceInfo {
public:
    SeSoundSourceInfo3DCircle(const char* pName) : SeSoundSourceInfo(pName) {}

    f32 mRadius = 0.0f;
    bool mIsCircleRotated = false;
};

class SeEmitterInfo {
public:
    static SeEmitterInfo* createInfo(const ByamlIter& rIter);
    static SeEmitterInfo* duplicateInfo(const SeEmitterInfo* pInfo);
    static s32 compareInfo(const SeEmitterInfo* pA, const SeEmitterInfo* pB);

    const char* mName = nullptr;
    const char* mJointName = nullptr;
    sead::Vector3f* mOffset = nullptr;
    SeSoundSourceInfo* mSoundSourceInfo = nullptr;
};

static_assert(sizeof(SeEmitterInfo) == 0x20);

class SeUserInfo {
public:
    static SeUserInfo* createInfo(const ByamlIter& rIter, const sead::SafeString& rName);
    static s32 compareInfo(const SeUserInfo* pA, const SeUserInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<SeEmitterInfo>* mEmitterInfoList = nullptr;
    AudioInfoList<SeActionInfo>* mActionInfoList = nullptr;
    AudioInfoList<SePlayInfo>* mPlayInfoList = nullptr;
};

static_assert(sizeof(SeUserInfo) == 0x20);
}  // namespace al
