#include "Project/Camera/CameraCollisionPartsFilter.hpp"

#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"

namespace al {

/**
 * Creates a filter for the camera move limit collision.
 */
CameraCollisionPartsFilter::CameraCollisionPartsFilter() {}

/**
 * Checks whether collision parts are not camera move limit collision.
 * @param rParts Collision parts to check.
 * @return Whether the parts have a special purpose other than "CameraMoveLimit".
 */
bool CameraCollisionPartsFilter::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSpecialPurpose) {
        return false;
    }

    return !isEqualString("CameraMoveLimit", rParts.mSpecialPurpose);
}

}  // namespace al
