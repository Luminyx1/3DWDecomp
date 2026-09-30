#include "Project/Camera/Holder/CameraSwitchInfo.hpp"

namespace al {

CameraSwitchInfo::CameraSwitchInfo() = default;

void CameraSwitchInfo::reset() {
    ticket = nullptr;
    interpoleStep = -1;
    isKeepPose = false;
    isSetNextPoseInfo = false;
}

}  // namespace al
