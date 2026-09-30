#pragma once

#include <basis/seadTypes.h>

namespace al {
class PtclSystem;

class EmitterSetResourceInfo {
public:
    EmitterSetResourceInfo(const char* pName, s32 resourceId, s32 emitterSetId, u32 flags,
                           bool isLoop, bool isInfinity);

    const char* mName;
    s32 mResourceId;
    s32 mEmitterSetId;
    u32 mFlags;
    bool mIsLoop;
    bool mIsInfinity;
};

static_assert(sizeof(EmitterSetResourceInfo) == 0x18);

class EmitterSetResourceInfoHolder {
public:
    EmitterSetResourceInfo* tryFindEffectResouceInfo(const char* pName) const;
    EmitterSetResourceInfo* findEffectResouceInfo(const char* pName) const;
    void createDataBase(PtclSystem* pPtclSystem);

    u8 _0[0x20];
};
}  // namespace al
