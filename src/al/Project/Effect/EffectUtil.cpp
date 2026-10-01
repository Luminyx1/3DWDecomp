#include "Project/Effect/EffectUtil.hpp"

#include <math/seadMathCalcCommon.h>
#include <nn/vfx/EmitterRes.h>

namespace {

sead::Color4f toColor(const nn::vfx::detail::ResAnimKey& rKey) {
    return *reinterpret_cast<const sead::Color4f*>(&rKey);
}

}  // namespace

namespace al {

void calcAnim8Key(sead::Color4f* pOut, const nn::vfx::detail::ResAnim8KeyParamSet& rParam,
                  f32 life, f32 frame) {
    s32 keyNum = rParam.keyNum;

    if (keyNum <= 0) {
        *pOut = sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    if (life <= 0.0f || keyNum == 1) {
        *pOut = toColor(rParam.keys[0]);
        return;
    }

    f32 rate;
    if (rParam.loop != 0) {
        rate = frame / rParam.loopRate;
        rate -= sead::Mathf::floor(rate);
    } else {
        rate = frame / life;
    }

    if (rate < 0.0f) {
        rate = 0.0f;
    } else if (rate > 1.0f) {
        rate = 1.0f;
    }

    if (rParam.keys[keyNum - 1].time <= rate) {
        *pOut = toColor(rParam.keys[keyNum - 1]);
        return;
    }

    f32 endTime = rParam.keys[0].time;
    if (rate <= endTime) {
        *pOut = toColor(rParam.keys[0]);
        return;
    }

    for (s32 i = 0; i < keyNum; i++) {
        f32 startTime = endTime;
        endTime = rParam.keys[i + 1].time;

        if (startTime <= rate && rate < endTime) {
            sead::Color4f start = toColor(rParam.keys[i]);
            sead::Color4f end = toColor(rParam.keys[i + 1]);
            pOut->setLerp(start, end, (rate - startTime) / (endTime - startTime));
            return;
        }
    }

    *pOut = sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f);
}

}  // namespace al
