#include "Library/Camera/CameraShaker.hpp"

#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

#include "Project/Base/StringUtil.hpp"

namespace {

struct ShakeParam {
    const char* name;
    s32 steps;
    f32 speed;
    f32 power;
};

const ShakeParam sShakeParams[] = {
    {"微弱", 25, 2.5f, 2.5f},     {"弱", 25, 2.5f, 4.0f},     {"中", 30, 3.0f, 8.0f},
    {"強", 45, 3.5f, 15.0f},      {"長い微弱", 60, 6.0f, 2.5f}, {"長い弱", 60, 6.0f, 4.0f},
};

}  // namespace

namespace al {

/**
 * Creates a shaker for a projection.
 * @param pProjection Projection offset by the shake.
 */
CameraShaker::CameraShaker(sead::PerspectiveProjection* pProjection) : mProjection(pProjection) {}

/**
 * Updates the shake and applies its offset to the projection.
 */
void CameraShaker::update() {
    if (mStep < 0) {
        return;
    }

    const ShakeParam& param = sShakeParams[mIndex];
    s32 steps = param.steps;

    if (steps <= mStep) {
        mStep = -1;
        mOffset = {0.0f, 0.0f};
        return;
    }

    f32 stepsF = steps;
    f32 wave = sead::Mathf::cos(sead::Mathf::deg2rad(param.speed * 360.0f / stepsF * mStep));
    f32 power = param.power * 0.001f * (steps - mStep) / stepsF;
    f32 offset = wave * power;
    mOffset.x = offset;
    mOffset.y = offset;
    mProjection->setOffset(mOffset);
    mStep++;
}

/**
 * Starts a shake if none is active.
 * @param index Shake index.
 */
void CameraShaker::startShake(s32 index) {
    if (mStep != -1) {
        return;
    }

    mStep = 0;
    mIndex = index;
}

/**
 * Starts a shake by its name if none is active.
 * @param pName Shake name.
 */
void CameraShaker::startShakeByString(const char* pName) {
    s32 index;

    if (isEqualString("微弱", pName)) {
        index = 0;
    } else if (isEqualString("弱", pName)) {
        index = 1;
    } else if (isEqualString("中", pName)) {
        index = 2;
    } else if (isEqualString("強", pName)) {
        index = 3;
    } else if (isEqualString("長い微弱", pName)) {
        index = 4;
    } else if (isEqualString("長い弱", pName)) {
        index = 5;
    } else {
        return;
    }

    startShake(index);
}

}  // namespace al
