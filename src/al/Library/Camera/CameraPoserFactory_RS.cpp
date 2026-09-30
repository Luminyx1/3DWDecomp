#include "Library/Camera/CameraPoserFactory_RS.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraPoser_RS.hpp"
#include "Library/Play/Camera/CameraPoserEntrance_RS.hpp"
#include "Library/Projection/Projection.hpp"
#include "Project/Camera/Info/CameraViewInfo.hpp"

namespace al {

/**
 * Creates a camera poser factory without entries.
 * @param pName Factory name.
 */
CameraPoserFactory_RS::CameraPoserFactory_RS(const char* pName) : Factory(pName) {}

/**
 * Creates the camera poser used for stage entrances.
 * @return New entrance camera poser.
 */
CameraPoserEntrance_RS* CameraPoserFactory_RS::createEntranceCameraPoser() const {
    return new CameraPoserEntrance_RS("Entrance");
}

}  // namespace al

namespace alCameraPoserFunction {

/**
 * Gets the index of the view the poser belongs to.
 * @param pPoser Camera poser.
 * @return View index.
 */
s32 getViewIndex(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getIndex();
}

/**
 * Gets the camera of the poser's view, which still holds the previous pose.
 * @param pPoser Camera poser.
 * @return View camera.
 */
const sead::LookAtCamera& getLookAtCamera(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getLookAtCam();
}

/**
 * Gets the projection of the poser's view.
 * @param pPoser Camera poser.
 * @return View projection.
 */
const al::Projection& getProjection(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getProjection();
}

/**
 * Gets the projection matrix of the poser's view.
 * @param pPoser Camera poser.
 * @return Projection matrix.
 */
const sead::Matrix44f& getProjectionMtx(const al::CameraPoser_RS* pPoser) {
    return *pPoser->getViewInfo()->getProjMtx();
}

/**
 * Gets the near clip distance of the poser's view.
 * @param pPoser Camera poser.
 * @return Near clip distance.
 */
f32 getNear(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getNear();
}

/**
 * Gets the far clip distance of the poser's view.
 * @param pPoser Camera poser.
 * @return Far clip distance.
 */
f32 getFar(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getFar();
}

/**
 * Gets the aspect ratio of the poser's view.
 * @param pPoser Camera poser.
 * @return Aspect ratio.
 */
f32 getAspect(const al::CameraPoser_RS* pPoser) {
    return pPoser->getViewInfo()->getAspect();
}

/**
 * Gets the camera position of the previous frame.
 * @param pPoser Camera poser.
 * @return Previous camera position.
 */
const sead::Vector3f& getPreCameraPos(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getPos();
}

/**
 * Gets the look-at position of the previous frame.
 * @param pPoser Camera poser.
 * @return Previous look-at position.
 */
const sead::Vector3f& getPreLookAtPos(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getAt();
}

/**
 * Gets the up direction of the previous frame.
 * @param pPoser Camera poser.
 * @return Previous up direction.
 */
const sead::Vector3f& getPreUpDir(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getUp();
}

/**
 * Gets the vertical field of view of the previous frame in degrees.
 * @param pPoser Camera poser.
 * @return Previous field of view in degrees.
 */
f32 getPreFovyDegree(const al::CameraPoser_RS* pPoser) {
    return sead::Mathf::rad2deg(getPreFovyRadian(pPoser));
}

/**
 * Gets the vertical field of view of the previous frame in radians.
 * @param pPoser Camera poser.
 * @return Previous field of view in radians.
 */
f32 getPreFovyRadian(const al::CameraPoser_RS* pPoser) {
    return getProjection(pPoser).getFovy();
}

}  // namespace alCameraPoserFunction
