#include "Project/Camera/Holder/CameraSwitchInfo.hpp"

namespace al {
/** @brief Creates an empty switch info. */
CameraSwitchInfo::CameraSwitchInfo()
    : _0(nullptr), _8(-1), _C(false), _D(false), _10(nullptr), _18(nullptr), _20(nullptr), _28(nullptr), _30(0) {}

/** @brief Clears the requested switch. */
void CameraSwitchInfo::reset() {
    _0 = nullptr;
    _8 = -1;
    _C = false;
    _D = false;
}
}  // namespace al
