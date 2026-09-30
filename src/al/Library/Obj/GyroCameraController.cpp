#include "Library/Obj/GyroCameraController.hpp"

namespace al {
/**
 * Creates the parameters with their default speeds and angle limits.
 */
GyroCameraControllerParam::GyroCameraControllerParam()
    : _0(0.0f), _4(0.0f), _8(10.0f), _c(45.0f), _10(-45.0f), _14(45.0f) {}
}  // namespace al
