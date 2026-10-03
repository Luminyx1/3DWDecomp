#include "Project/Joint/KeyPose.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"

namespace al {

/**
 * Constructs an identity key pose at the origin.
 */
KeyPose::KeyPose() = default;

/**
 * Reads the pose from a placement and keeps a copy of the placement.
 * @param rInfo Placement to read from.
 */
void KeyPose::init(const PlacementInfo& rInfo) {
    tryGetQuat(&mQuat, rInfo);
    tryGetTrans(&mTrans, rInfo);
    mPlacementInfo = new PlacementInfo(rInfo);
}

}  // namespace al
