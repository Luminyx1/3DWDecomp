#pragma once

#include <container/seadSafeArray.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {

/**
 * Watches the GPU load and lowers (or restores) the rendering quality level step by step.
 */
class GraphicsQualityController : public NerveExecutor {
public:
    struct QualityLevel {
        s32 level;
        f32 rate;
    };

    GraphicsQualityController(f32* pRecoverPercents);

    void setRecoverPercents(f32* pRecoverPercents);

    void exeWait();
    void exeReduce();
    void exeRecover();

    f32 getRate(s32 level) const {
        volatile s32 index = level;

        if (!mIsEnable)
            return 1.0f;
        return mQualityLevels[index].rate;
    }

    bool isEnable() const { return mIsEnable; }

    void setEnable(bool isEnable) { mIsEnable = isEnable; }

    void setReduceQualityPercentage(f32 percentage) { mReduceQualityPercentage = percentage; }

private:
    sead::SafeArray<QualityLevel, 6> mQualityLevels;
    f32 mReduceQualityPercentage = 93.0f;
    s32 mLevel = -1;
    bool mIsEnable = true;
    f32* mRecoverPercents;
};

static_assert(sizeof(GraphicsQualityController) == 0x58);

}  // namespace al
