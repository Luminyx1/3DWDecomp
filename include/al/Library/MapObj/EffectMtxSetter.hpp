#pragma once

#include <math/seadMatrix.h>

namespace al {
class ByamlIter;
class LiveActor;

class EffectMtxInfo {
public:
    EffectMtxInfo();

    void init(const ByamlIter& rIter);
    void setMtxPtr(LiveActor* pActor, const sead::Matrix34f* pMtx);

    const char* mMtxName = nullptr;
    const sead::Matrix34f* mMtx = nullptr;
    const char** mEffectNames = nullptr;
    s32 mEffectNum = 0;
};

static_assert(sizeof(EffectMtxInfo) == 0x20);

class EffectMtxSetter {
public:
    EffectMtxSetter(LiveActor* pActor);

    void init(const ByamlIter& rIter);
    void setMtxPtr(const sead::Matrix34f* pMtx, const char* pMtxName);
    EffectMtxInfo* tryFindEffectMtxInfo(const char* pMtxName);

    LiveActor* mActor;
    EffectMtxInfo* mInfos = nullptr;
    s32 mInfoNum = 0;
};

static_assert(sizeof(EffectMtxSetter) == 0x18);

EffectMtxSetter* tryCreateEffectMtxSetter(LiveActor* pActor, const char* pName);
}  // namespace al
