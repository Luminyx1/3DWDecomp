#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraStartInfo.hpp"
#include "Library/Play/Camera/CameraPoserProgramable_RS.hpp"

namespace al {

/**
 * Constructs a programmable poser that keeps the collider state of the previous camera.
 * @param pPos Camera position source, may be null.
 * @param pAt Look-at position source, may be null.
 * @param pUp Up direction source, may be null.
 */
CameraPoserProgramableKeepColliderPreCamera::CameraPoserProgramableKeepColliderPreCamera(
    const sead::Vector3f* pPos, const sead::Vector3f* pAt, const sead::Vector3f* pUp)
    : CameraPoserProgramable_RS(pPos, pAt, pUp) {}

/**
 * Initializes the arrow collider.
 */
void CameraPoserProgramableKeepColliderPreCamera::init() {
    alCameraPoserFunction::initCameraArrowCollider(this);
}

/**
 * Takes over whether the previous camera had its collider disabled.
 * @param rInfo Camera start info.
 */
void CameraPoserProgramableKeepColliderPreCamera::start(const CameraStartInfo& rInfo) {
    if (rInfo.isInvalidCollidePreCamera) {
        alCameraPoserFunction::invalidateCollider(this);
    } else {
        alCameraPoserFunction::validateCollider(this);
    }
}

}  // namespace al
