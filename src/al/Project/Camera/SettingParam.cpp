#include "Project/Camera/SettingParam.hpp"

#include <nn/oe.h>

#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/ControlAngleParam.hpp"
#include "Project/Camera/CameraPoser.hpp"

namespace al {
/**
 * Creates the settings with their default values.
 */
SettingParam::SettingParam()
    : _0(200.0f), _4(100.0f), _8(200.0f), _C(1.5f), _10(60), _14(60), _18(20.0f), _1C(40.0f), _20(200.0f),
      _24(150.0f), _28(20.0f), _2C(0.0f), _30(1.0f), _34(false), _35(false), _36(false) {}

/**
 * Creates a poser at the origin that looks along the z axis.
 */
CameraPoser::CameraPoser() = default;

/**
 * Leaves the snapshot mode and resets the orientation of album screenshots.
 */
void CameraPoser::endSnapshotMode() {
    mIsSnapshotMode = false;
    nn::oe::SetAlbumImageOrientation(nn::album::ImageOrientation_None);
}

/**
 * Reads the common camera parameters.
 * @param pIter The camera parameters.
 */
void CameraPoser::loadParam(const ByamlIter* pIter) {
    pIter->tryGetFloatByKey(&mFovyDegree, "Fovy");
    pIter->tryGetIntByKey(&mInterpolationFrame, "InterpolationFrame");

    ByamlIter angleIter;
    if (!pIter->tryGetIterByKey(&angleIter, "ControlAngleParam")) {
        return;
    }

    angleIter.tryGetBoolByKey(&mControlAngleParam->mIsInvalidControl, "IsInvalidControl");
    if (mControlAngleParam->mIsInvalidControl) {
        return;
    }

    mControlAngleParam->mIsValid = true;
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleVLimitMin, "AngleVLimitMin");
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleVLimitMax, "AngleVLimitMax");
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleHLimitMin, "AngleHLimitMin");
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleHLimitMax, "AngleHLimitMax");
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleVStep, "AngleVStep");
    angleIter.tryGetFloatByKey(&mControlAngleParam->mAngleHStep, "AngleHStep");
}
}  // namespace al
