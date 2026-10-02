#include "Library/Play/Camera/CameraPoserShooterSingle.hpp"

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Camera/SimpleCameraShooter.hpp"

namespace al {

/**
 * Constructs a shooter camera that is tilted down slightly and placed behind the target.
 * @param pName Poser name.
 */
CameraPoserShooterSingle::CameraPoserShooterSingle(const char* pName) : CameraPoser_RS(pName) {
    mShooter = new SimpleCameraShooter(pName);
    mShooter->rotatePitch(-30.0f);
    mShooter->setDistance(2000.0f);
}

/**
 * Loads the target offset.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserShooterSingle::loadParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&mOffset, rIter, "Offset");
}

/**
 * Moves the shooter to the target, rotates it with the stick and takes over its pose.
 */
void CameraPoserShooterSingle::update() {
    sead::Vector3f targetTrans = sead::Vector3f::zero;
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    mShooter->setTargetPos(targetTrans + mOffset);

    sead::Vector2f stick;
    alCameraPoserFunction::calcCameraRotateStick(&stick, this);
    mShooter->rotateYaw(stick.x * -100.0f);
    mShooter->rotatePitch(stick.y * -100.0f);

    sead::LookAtCamera camera;
    mShooter->applyCameraWithCollision(&camera, this);
    camera.updateViewMatrix();
    mAt.set(camera.getAt());
    mEye.set(camera.getPos());
}

}  // namespace al
