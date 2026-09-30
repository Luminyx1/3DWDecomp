#include "Library/Camera/CameraTriangleFilter.hpp"

#include "Library/Collision/CollisionPartsKeeperUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"

namespace al {

static bool isHiddenHostTriangle(const Triangle& rTriangle) {
    const CollisionParts* parts = rTriangle.mCollisionParts;
    if (!parts->mSensor) {
        return false;
    }
    LiveActor* host = parts->getConnectedHost();
    return isExistModel(host) && isHideModel(host) && !isCameraCode("NoThroughAlways", rTriangle);
}

/**
 * Returns whether the camera passes through a triangle.
 * @param rTriangle Triangle to check.
 * @return Whether the triangle is ignored by the camera.
 */
bool CameraTriangleFilter::isInvalidTriangle(const Triangle& rTriangle) const {
    if (isCameraCode("InvalidThrough", rTriangle)) {
        return false;
    }
    if (isHiddenHostTriangle(rTriangle)) {
        return true;
    }
    return isCameraCode("Through", rTriangle);
}

/**
 * Returns whether the camera passes through a triangle, ignoring everything but ceilings.
 * @param rTriangle Triangle to check.
 * @return Whether the triangle is ignored by the camera.
 */
bool CameraTriangleFilterOnlyCeiling::isInvalidTriangle(const Triangle& rTriangle) const {
    if (!isCeilingPolygon(*rTriangle.getNormal(0), {0.0f, -1.0f, 0.0f})) {
        return true;
    }
    return CameraTriangleFilter::isInvalidTriangle(rTriangle);
}

/**
 * Returns whether the subjective camera passes through a triangle.
 * @param rTriangle Triangle to check.
 * @return Whether the triangle is ignored by the camera.
 */
bool SubjectiveCameraTriangleFilter::isInvalidTriangle(const Triangle& rTriangle) const {
    if (isCameraCode("InvalidThrough", rTriangle)) {
        return false;
    }
    if (isHiddenHostTriangle(rTriangle)) {
        return true;
    }
    if (isFloorPolygon(*rTriangle.getNormal(0), -sead::Vector3f::ey)) {
        return true;
    }
    if (mIsIgnoreThrough) {
        return false;
    }
    return isCameraCode("Through", rTriangle);
}

}  // namespace al
