#include "Project/Joint/KeyPose.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {

KeyPose::KeyPose() = default;

void KeyPose::init(const PlacementInfo& info) {
    tryGetQuat(&mQuat, info);
    tryGetTrans(&mTrans, info);
    mPlacementInfo = new PlacementInfo(info);
}

}  // namespace al
