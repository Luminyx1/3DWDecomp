#pragma once

#include <basis/seadTypes.h>

namespace al {

struct AnimResInfo {
    AnimResInfo();

    s32 getFrameMax() const;
    bool isLoop() const;

    const char* name = nullptr;
    void* resAnim = nullptr;
    f32 frameMax = 0.0f;
    bool isLoopAnim = false;
};

static_assert(sizeof(AnimResInfo) == 0x18);

class AnimInfoTable {
public:
    AnimInfoTable(s32 maxInfos);

    void add(const char* pName, void* pResAnim, f32 frameMax, bool isLoop);
    const AnimResInfo* findAnimInfo(const char* pName) const;
    const AnimResInfo* tryFindAnimInfo(const char* pName) const;
    void sort();

    s32 getInfoCount() const { return mInfoCount; }
    const AnimResInfo& getResInfo(s32 index) const { return mResInfos[index]; }

private:
    s32 mInfoCount = 0;
    AnimResInfo* mResInfos = nullptr;
    bool mIsSorted = false;
};

static_assert(sizeof(AnimInfoTable) == 0x18);

}  // namespace al
