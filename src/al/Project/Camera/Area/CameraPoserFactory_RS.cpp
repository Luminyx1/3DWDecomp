#include "Project/Camera/Area/CameraPoserFactory_RS.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Projection/Projection.hpp"
#include "Project/Camera/Area/CameraPoserFunction.hpp"
#include "Project/Camera/CameraPoserEntrance_RS.hpp"
#include "Project/Camera/Info/CameraViewInfo.hpp"
#include "Project/Camera/Main/CameraPoser_RS.hpp"

namespace al {
/**
 * @brief Creates a factory without any creator functions.
 * @param pName The name of the factory.
 */
CameraPoserFactory_RS::CameraPoserFactory_RS(const char* pName) : Factory(pName) {}

/**
 * @brief Creates the camera that is used when entering a stage.
 * @return The new entrance camera.
 */
CameraPoser_RS* CameraPoserFactory_RS::createEntranceCameraPoser() const {
    return new CameraPoserEntrance_RS("Entrance");
}
}  // namespace al

namespace alCameraPoserFunction {
/**
 * @brief Gets the index of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The view index.
 */
s32 getViewIndex(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->mIndex;
}

/**
 * @brief Gets the camera of the view the poser belongs to, which still holds the previous pose.
 * @param pPoser The camera poser.
 * @return The camera of the view.
 */
const sead::LookAtCamera& getLookAtCamera(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->mLookAtCam;
}

/**
 * @brief Gets the projection of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The projection of the view.
 */
const al::Projection& getProjection(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->mProjection;
}

/**
 * @brief Gets the projection matrix of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The projection matrix.
 */
const sead::Matrix44f& getProjectionMtx(const al::CameraPoser_RS* pPoser) {
    return *pPoser->mViewInfo->getProjMtx();
}

/**
 * @brief Gets the near clip distance of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The near clip distance.
 */
f32 getNear(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->getNear();
}

/**
 * @brief Gets the far clip distance of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The far clip distance.
 */
f32 getFar(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->getFar();
}

/**
 * @brief Gets the aspect ratio of the view the poser belongs to.
 * @param pPoser The camera poser.
 * @return The aspect ratio.
 */
f32 getAspect(const al::CameraPoser_RS* pPoser) {
    return pPoser->mViewInfo->getAspect();
}

/**
 * @brief Gets the camera position of the previous frame.
 * @param pPoser The camera poser.
 * @return The previous camera position.
 */
const sead::Vector3f& getPreCameraPos(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getPos();
}

/**
 * @brief Gets the look-at position of the previous frame.
 * @param pPoser The camera poser.
 * @return The previous look-at position.
 */
const sead::Vector3f& getPreLookAtPos(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getAt();
}

/**
 * @brief Gets the up direction of the previous frame.
 * @param pPoser The camera poser.
 * @return The previous up direction.
 */
const sead::Vector3f& getPreUpDir(const al::CameraPoser_RS* pPoser) {
    return getLookAtCamera(pPoser).getUp();
}

/**
 * @brief Gets the field of view of the previous frame in degrees.
 * @param pPoser The camera poser.
 * @return The previous vertical field of view in degrees.
 */
f32 getPreFovyDegree(const al::CameraPoser_RS* pPoser) {
    return sead::Mathf::rad2deg(getProjection(pPoser).getFovy());
}

/**
 * @brief Gets the field of view of the previous frame in radians.
 * @param pPoser The camera poser.
 * @return The previous vertical field of view in radians.
 */
f32 getPreFovyRadian(const al::CameraPoser_RS* pPoser) {
    return getProjection(pPoser).getFovy();
}
}  // namespace alCameraPoserFunction
