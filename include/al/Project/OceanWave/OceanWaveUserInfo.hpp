#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;
class LiveActor;

class OceanWaveInfo {
public:
    static OceanWaveInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const OceanWaveInfo* pA, const OceanWaveInfo* pB);

    const char* mName = nullptr;
    const char* mJointName = nullptr;
    sead::Vector3f mPosOffset;
    f32 mSize = 0.0f;
    f32 mSpeed = 0.0f;
    f32 mTime = 0.0f;
    f32 mAmp = 0.0f;
    f32 mLen = 0.0f;
};

static_assert(sizeof(OceanWaveInfo) == 0x30);

class OceanWavePlayInfo {
public:
    static OceanWavePlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const OceanWavePlayInfo* pA, const OceanWavePlayInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<OceanWaveInfo>* mOceanWaveInfoList = nullptr;
    const char* mRequestKeeperName = nullptr;
};

static_assert(sizeof(OceanWavePlayInfo) == 0x18);

class OceanWavePlayInfoInAction {
public:
    OceanWavePlayInfoInAction();
    OceanWavePlayInfoInAction(const OceanWavePlayInfoInAction& rOther);
    OceanWavePlayInfoInAction& operator=(const OceanWavePlayInfoInAction& rOther);

    static OceanWavePlayInfoInAction* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const OceanWavePlayInfoInAction* pA,
                           const OceanWavePlayInfoInAction* pB);

    const char* mName = nullptr;
    f32 mStartFrame = 0.0f;
    f32 mEndFrame = 0.0f;
};

static_assert(sizeof(OceanWavePlayInfoInAction) == 0x10);

class OceanWaveActionInfo {
public:
    OceanWaveActionInfo();
    OceanWaveActionInfo(const OceanWaveActionInfo& rOther);
    OceanWaveActionInfo& operator=(const OceanWaveActionInfo& rOther);

    static OceanWaveActionInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const OceanWaveActionInfo* pA, const OceanWaveActionInfo* pB);

    const char* mName;
    AudioInfoList<OceanWavePlayInfoInAction>* mPlayInfoList;
};

static_assert(sizeof(OceanWaveActionInfo) == 0x10);

class OceanWaveUserInfo {
public:
    static OceanWaveUserInfo* createInfo(const ByamlIter& rIter, const sead::SafeString& rName);
    static s32 compareInfo(const OceanWaveUserInfo* pA, const OceanWaveUserInfo* pB);

    const char* mName = nullptr;
    const char* mParentName = nullptr;
    AudioInfoList<OceanWaveActionInfo>* mActionInfoList = nullptr;
    AudioInfoList<OceanWavePlayInfo>* mPlayInfoList = nullptr;
};

static_assert(sizeof(OceanWaveUserInfo) == 0x20);

void startOceanWave(LiveActor* pActor, const char* pName);
}  // namespace al
