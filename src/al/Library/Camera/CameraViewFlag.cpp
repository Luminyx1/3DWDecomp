#include "Library/Camera/CameraViewFlag.hpp"

namespace al {

/**
 * Creates a view flag set with every flag cleared.
 */
CameraViewFlag::CameraViewFlag() = default;

/**
 * Clears every flag.
 */
void CameraViewFlag::resetAllFlag() {
    mIsInvalidCameraBlur = false;
}

}  // namespace al
