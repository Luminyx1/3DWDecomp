#pragma once

#include <basis/seadTypes.h>

namespace al {
enum GraphicsAreaParamType : s64 {};

class CurrentGraphicsAreaParam {
public:
    CurrentGraphicsAreaParam();

    const char* mParamName = nullptr;
    const char* mPrevParamName = nullptr;
    f32 mRate = 1.0f;
    s32 _14 = 0;
    bool mIsLerp = false;
    bool mIsNoParam = false;
    s32 mPriority = -1;
};

static_assert(sizeof(CurrentGraphicsAreaParam) == 0x20);

class GraphicsAreaDirector {
public:
    s32 getGraphicsAreaNum() const;
    void getCurrentGraphicsAreaParam(CurrentGraphicsAreaParam* pParam,
                                     GraphicsAreaParamType type) const;

    bool isLerpPaused() const { return mIsLerpPaused; }

private:
    u8 _0[0x290];
    bool mIsLerpPaused;
};
}  // namespace al
