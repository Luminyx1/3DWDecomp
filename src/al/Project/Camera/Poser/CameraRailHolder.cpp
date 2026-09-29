#include "Project/Camera/Poser/CameraRailHolder.hpp"

#include "Project/Camera/Poser/CameraRailInfo.hpp"

namespace al {
/** @brief Creates a holder with room for 32 rails. */
CameraRailHolder::CameraRailHolder() {
    mRailInfos = new CameraRailInfo[32];
}

/**
 * @brief Adds a rail.
 * @param pRail The rail to add.
 * @param isValid Whether the rail can be used.
 */
void CameraRailHolder::setCameraRail(CameraRail* pRail, bool isValid) {
    mRailInfos[mNumRails].mRail = pRail;
    mRailInfos[mNumRails].mIsValid = isValid;
    mNumRails++;
}

/**
 * @brief Gets a rail.
 * @param index The index of the rail.
 * @return The rail.
 */
CameraRail* CameraRailHolder::getCameraRail(s32 index) const {
    return mRailInfos[index].mRail;
}

/**
 * @brief Enables or disables a rail.
 * @param index The index of the rail.
 * @param isValid Whether the rail can be used.
 */
void CameraRailHolder::setCameraRailValidFlag(s32 index, bool isValid) {
    mRailInfos[index].mIsValid = isValid;
}

/**
 * @brief Checks whether a rail can be used.
 * @param index The index of the rail.
 * @return True if the rail is enabled.
 */
bool CameraRailHolder::isCameraRailValid(s32 index) const {
    return mRailInfos[index].mIsValid;
}

/** @brief Creates an info that is not bound to any rail yet. */
CameraRailInfo::CameraRailInfo() : mRail(nullptr), mIsValid(true) {}
}  // namespace al
