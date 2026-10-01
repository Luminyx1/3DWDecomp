#include "Project/Effect/EffectCameraHolder.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Project/Camera/Core/IUseCameraDirector.hpp"

namespace al {
namespace {
class EffectCameraUser : public IUseCamera {
public:
    EffectCameraUser(SceneCameraInfo* pSceneCameraInfo) : mSceneCameraInfo(pSceneCameraInfo) {}

    SceneCameraInfo* getSceneCameraInfo() const override { return mSceneCameraInfo; }

private:
    SceneCameraInfo* mSceneCameraInfo;
};
}  // namespace

/**
 * Creates a camera holder without a scene camera.
 */
EffectCameraHolder::EffectCameraHolder() : mSceneCameraInfo(nullptr) {}

/**
 * Sets the scene camera used by effects.
 * @param pInfo Scene camera info.
 */
void EffectCameraHolder::setSceneCameraInfo(SceneCameraInfo* pInfo) {
    mSceneCameraInfo = pInfo;
}

/**
 * Gets the camera view matrix.
 * @return Pointer to the view matrix.
 */
const sead::Matrix34f* EffectCameraHolder::getViewMtxPtr() const {
    EffectCameraUser user(mSceneCameraInfo);
    return getCameraViewMtxPtr(&user);
}

/**
 * Gets the camera's vertical field of view.
 * @return Fovy in degrees.
 */
f32 EffectCameraHolder::getFovy() const {
    EffectCameraUser user(mSceneCameraInfo);
    return getCameraFovyDegree(&user);
}

/**
 * Gets the camera position.
 * @return Camera position.
 */
const sead::Vector3f& EffectCameraHolder::getCameraPos() const {
    EffectCameraUser user(mSceneCameraInfo);
    return al::getCameraPos(&user);
}

/**
 * Projects a world position to the screen of the main view.
 * @param pScreenPos Receives the screen position.
 * @param rPos World position.
 */
void EffectCameraHolder::calcScreenPosFromWorldPos(sead::Vector2f* pScreenPos,
                                                   const sead::Vector3f& rPos) const {
    EffectCameraUser user(mSceneCameraInfo);
    al::calcScreenPosFromWorldPos(pScreenPos, &user, rPos, 0);
}

/**
 * Computes the scale that follows the camera's fovy relative to a 40 degree fovy.
 * @return Follow scale.
 */
f32 EffectCameraHolder::calcFollowScaleByFovy() const {
    EffectCameraUser user(mSceneCameraInfo);
    return tanf(getCameraFovyRadian(&user) * 0.5f) / 0.36397022f;
}

/**
 * Computes the far clip rate from the camera's fovy relative to a 40 degree fovy.
 * @return Far clip rate.
 */
f32 EffectCameraHolder::calcFarClipRateByFovy() const {
    EffectCameraUser user(mSceneCameraInfo);
    return 0.36397022f / tanf(getCameraFovyRadian(&user) * 0.5f);
}

/**
 * Makes a matrix at the camera that faces a position.
 * @param pMtx Receives the matrix.
 * @param rPos Position to face.
 * @return Whether the matrix could be made.
 */
bool EffectCameraHolder::tryMakeBillboardMtx(sead::Matrix34f* pMtx, const sead::Vector3f& rPos) const {
    pMtx->setInverse(*getViewMtxPtr());

    sead::Vector3f up;
    pMtx->getBase(up, 1);
    sead::Vector3f front = pMtx->getTranslation() - rPos;

    if (!tryNormalizeOrZero(&front)) {
        return false;
    }

    if (isParallelDirection(front, up)) {
        return false;
    }

    makeMtxFrontUp(pMtx, front, up);
    return true;
}

/**
 * Makes a Y-axis billboard matrix at the camera that faces a position.
 * @param pMtx Receives the matrix.
 * @param rPos Position to face.
 * @return Whether the matrix could be made.
 */
bool EffectCameraHolder::tryMakeYBillboardMtx(sead::Matrix34f* pMtx, const sead::Vector3f& rPos) const {
    pMtx->setInverse(*getViewMtxPtr());

    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = pMtx->getTranslation() - rPos;

    if (!tryNormalizeOrZero(&front)) {
        return false;
    }

    if (isParallelDirection(front, up)) {
        return false;
    }

    makeMtxUpFront(pMtx, up, front);
    return true;
}

/**
 * Makes a matrix at the camera facing the camera's front direction.
 * @param pMtx Receives the matrix.
 * @return Whether the matrix could be made.
 */
bool EffectCameraHolder::tryMakeCameraFrontMtx(sead::Matrix34f* pMtx) const {
    pMtx->setInverse(*getViewMtxPtr());

    sead::Vector3f up;
    pMtx->getBase(up, 1);
    sead::Vector3f front;
    pMtx->getBase(front, 2);

    if (!tryNormalizeOrZero(&front)) {
        return false;
    }

    if (isParallelDirection(front, up)) {
        return false;
    }

    makeMtxFrontUp(pMtx, front, up);
    return true;
}
}  // namespace al
