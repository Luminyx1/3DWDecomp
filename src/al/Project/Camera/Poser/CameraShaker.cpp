#include "Project/Camera/Poser/CameraShaker.hpp"

#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
struct ShakeInfo {
    const char* mName;  // _0
    s32 mFrame;         // _8
    f32 mCycleNum;      // _C
    f32 mAmplitude;     // _10
};

const ShakeInfo sShakeInfo[] = {
    {"微弱", 25, 2.5f, 2.5f},     {"弱", 25, 2.5f, 4.0f},       {"中", 30, 3.0f, 8.0f},
    {"強", 45, 3.5f, 15.0f},      {"長い微弱", 60, 6.0f, 2.5f}, {"長い弱", 60, 6.0f, 4.0f},
};
}  // namespace

/**
 * @brief Creates a shaker that is not shaking.
 * @param pProjection The projection whose offset is moved.
 */
CameraShaker::CameraShaker(sead::PerspectiveProjection* pProjection) : mProjection(pProjection) {}

/** @brief Advances the shake by one frame, moving the projection offset along a fading cosine wave. */
void CameraShaker::update() {
    if (mFrame < 0) {
        return;
    }

    const ShakeInfo& info = sShakeInfo[mType];
    if (info.mFrame <= mFrame) {
        mFrame = -1;
        mOffset.x = 0.0f;
        mOffset.y = 0.0f;
        return;
    }

    f32 frameNum = info.mFrame;
    f32 wave = cosf(sead::Mathf::deg2rad(info.mCycleNum * 360.0f / frameNum * mFrame));
    f32 offset = wave * (info.mAmplitude * 0.001f * (info.mFrame - mFrame) / frameNum);
    mOffset.x = offset;
    mOffset.y = offset;
    mProjection->setOffset(mOffset);
    mFrame++;
}

/**
 * @brief Starts a shake unless one is already running.
 * @param type The index of the shake type.
 */
void CameraShaker::startShake(s32 type) {
    if (mFrame != -1) {
        return;
    }
    mFrame = 0;
    mType = type;
}

/**
 * @brief Starts a shake by the name of its type.
 * @param pName The name of the shake type: 微弱 (very weak), 弱 (weak), 中 (medium), 強 (strong), 長い微弱 (long very
 * weak) or 長い弱 (long weak).
 */
void CameraShaker::startShakeByString(const char* pName) {
    s32 type;
    if (isEqualString("微弱", pName)) {
        type = 0;
    }
    else if (isEqualString("弱", pName)) {
        type = 1;
    }
    else if (isEqualString("中", pName)) {
        type = 2;
    }
    else if (isEqualString("強", pName)) {
        type = 3;
    }
    else if (isEqualString("長い微弱", pName)) {
        type = 4;
    }
    else if (isEqualString("長い弱", pName)) {
        type = 5;
    }
    else {
        return;
    }
    startShake(type);
}
}  // namespace al
