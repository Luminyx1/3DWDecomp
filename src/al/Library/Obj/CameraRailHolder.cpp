#include "Library/Obj/CameraRailHolder.hpp"

namespace al {
/**
 * Constructs a camera rail holder with room for 32 rails.
 */
CameraRailHolder::CameraRailHolder() {
    mCameraRailInfos = new CameraRailInfo[32];
}

/**
 * Adds a camera rail.
 * @param pRail camera rail
 * @param isValid whether the rail is valid
 */
void CameraRailHolder::setCameraRail(CameraRail* pRail, bool isValid) {
    mCameraRailInfos[mCameraRailNum].mRail = pRail;
    mCameraRailInfos[mCameraRailNum].mIsValid = isValid;
    mCameraRailNum++;
}

/**
 * Gets a camera rail.
 * @param index rail index
 * @return camera rail
 */
CameraRail* CameraRailHolder::getCameraRail(s32 index) const {
    return mCameraRailInfos[index].mRail;
}

/**
 * Sets whether a camera rail is valid.
 * @param index rail index
 * @param isValid whether the rail is valid
 */
void CameraRailHolder::setCameraRailValidFlag(s32 index, bool isValid) {
    mCameraRailInfos[index].mIsValid = isValid;
}

/**
 * Checks if a camera rail is valid.
 * @param index rail index
 * @return whether the rail is valid
 */
bool CameraRailHolder::isCameraRailValid(s32 index) const {
    return mCameraRailInfos[index].mIsValid;
}

/**
 * Constructs an empty valid camera rail info.
 */
CameraRailInfo::CameraRailInfo() = default;
}  // namespace al
