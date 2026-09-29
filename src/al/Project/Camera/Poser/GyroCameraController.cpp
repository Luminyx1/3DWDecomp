#include "Project/Camera/Poser/GyroCameraControllerParam.hpp"

namespace al {
/** @brief Creates the parameters with their default speeds and angle limits. */
GyroCameraControllerParam::GyroCameraControllerParam()
    : _0(0.0f), _4(0.0f), _8(10.0f), _C(45.0f), _10(-45.0f), _14(45.0f) {}
}  // namespace al
