#include "Library/Play/Camera/CameraPoserFixPoint.hpp"

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Creates a camera at a fixed point that looks at the target.
 * @param pName Camera name. "その場定点" uses the previous camera position.
 */
CameraPoserFixPoint::CameraPoserFixPoint(const char* pName) : CameraPoser_RS(pName) {
    if (isEqualString(pName, "その場定点")) {
        mIsUsePrePoserPos = true;
    }
}

/**
 * Initializes angle swing, the snapshot controller and, unless the camera fully follows the
 * target, the vertical absorber.
 */
void CameraPoserFixPoint::init() {
    alCameraPoserFunction::initAngleSwing(this);
    alCameraPoserFunction::initSnapShotCameraCtrlZoomAutoReset(this);
    if (isEqualString(getName(), "完全追従定点")) {
        return;
    }

    alCameraPoserFunction::initCameraVerticalAbsorberNoCameraPosAbsorb(this);
}

/**
 * Loads the camera position, height offset and distance settings.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFixPoint::loadParam(const ByamlIter& rIter) {
    if (!tryGetByamlBool(&mIsUsePrePoserPos, rIter, "IsUsePrePoserPos") || !mIsUsePrePoserPos) {
        tryGetByamlV3f(&mCameraPos, rIter, "CameraPos");
    }

    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlBool(&mIsKeepDistanceFromLookAt, rIter, "IsKeepDistanceFromLookAt");
    if (mIsKeepDistanceFromLookAt) {
        tryGetByamlF32(&mKeepDistance, rIter, "KeepDistance");
    }
}

/**
 * Takes over the previous camera position when requested and updates the pose.
 * @param rInfo Start info.
 */
void CameraPoserFixPoint::start(const CameraStartInfo& rInfo) {
    if (mIsUsePrePoserPos) {
        alCameraPoserFunction::multVecInvZone(
            &mCameraPos, alCameraPoserFunction::getLookAtCamera(this).getPos(), this);
    }

    update();
}

/**
 * Places the camera at the fixed point and looks at the offset target position.
 */
void CameraPoserFixPoint::update() {
    mEye.set(mCameraPos);
    mEye.setMul(mViewMtx, mEye);
    mUp.set(sead::Vector3f::ey);
    alCameraPoserFunction::setLookAtPosToTargetAddOffset(this, {0.0f, mOffsetY, 0.0f});
}

/**
 * Keeps a fixed distance between camera and look-at position when enabled.
 * @param pCamera Camera to modify.
 */
void CameraPoserFixPoint::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    if (!mIsKeepDistanceFromLookAt) {
        return;
    }

    sead::Vector3f dir = pCamera->getPos() - pCamera->getAt();
    f32 distance = mKeepDistance;
    f32 length = dir.length();
    if (length > 0.0f) {
        dir *= distance / length;
    }

    pCamera->setPos(dir + pCamera->getAt());
}

}  // namespace al
