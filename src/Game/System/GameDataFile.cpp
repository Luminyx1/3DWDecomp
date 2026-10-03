#include "System/GameDataFile.hpp"

u8 GameDataFile::sCameraSettings;

/**
 * @brief Read the packed camera settings.
 * @return The camera-option bits stored in the low byte.
 */
u16 GameDataFile::getOptions() { return sCameraSettings; }

/**
 * @brief Replace the packed camera settings.
 * @param options Camera-option bits; only the low eight bits are retained.
 */
void GameDataFile::setOptions(u16 options) { sCameraSettings = options; }

/**
 * @brief Restore the default camera direction settings.
 */
void GameDataFile::initCameraSettings() { sCameraSettings = 0; }

/**
 * @brief Check whether the camera axis is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getCameraReverseVertical() { return (sCameraSettings & 1) != 0; }

/**
 * @brief Change the camera axis reversal option.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setCameraReverseVertical(GameDataHolder* pHolder, bool reverse) {
    sCameraSettings = reverse ? sCameraSettings | 1 : sCameraSettings & ~1;
}

/**
 * @brief Check whether the camera axis is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getCameraReverseHorizontal() { return (sCameraSettings & 2) != 0; }

/**
 * @brief Change the camera axis reversal option.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setCameraReverseHorizontal(GameDataHolder* pHolder, bool reverse) {
    sCameraSettings = reverse ? sCameraSettings | 2 : sCameraSettings & ~2;
}
