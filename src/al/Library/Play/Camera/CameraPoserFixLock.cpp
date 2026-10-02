#include "Library/Play/Camera/CameraPoserFixLook.hpp"

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {

sead::Vector3f sLookAtPos = sead::Vector3f::zero;

}  // namespace

namespace al {

/**
 * Creates a camera looking at a fixed position from the previous camera position.
 * @param pName Camera name.
 */
CameraPoserFixLook::CameraPoserFixLook(const char* pName)
    : CameraPoser_RS(pName), mTargetTrans(&sLookAtPos) {
    mUp.set(sead::Vector3f::ey);
}

/**
 * Initializes the arrow collider.
 */
void CameraPoserFixLook::init() {
    alCameraPoserFunction::initCameraArrowCollider(this);
}

/**
 * Starts looking at the target from the previous camera pose.
 * @param rInfo Camera start info.
 */
void CameraPoserFixLook::start(const CameraStartInfo& rInfo) {
    mAt.set(*mTargetTrans);
    mEye.set(alCameraPoserFunction::getPreCameraPos(this));
    mUp.set(alCameraPoserFunction::getPreUpDir(this));
    mFovyDegree = alCameraPoserFunction::getPreFovyDegree(this);
}

/**
 * Loads the look at position.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFixLook::loadParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&sLookAtPos, rIter, "LookAtPos");
}

}  // namespace al
