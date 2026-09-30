#include "Library/Clipping/ClippingJudge.hpp"

#include <gfx/seadCamera.h>

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Clipping/FrustumRadar.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/Clipping/ClippingFarAreaObserver.hpp"

namespace al {
/**
 * Constructs the clipping judge.
 * @param pFarAreaObserver far clip area observer
 * @param pSceneCameraInfo scene camera info
 * @param pCameraDirector camera director, or nullptr
 */
ClippingJudge::ClippingJudge(const ClippingFarAreaObserver* pFarAreaObserver,
                             SceneCameraInfo* pSceneCameraInfo,
                             CameraDirector_RS* pCameraDirector)
    : mFarAreaObserver(pFarAreaObserver), mSceneCameraInfo(pSceneCameraInfo),
      mCameraDirector(pCameraDirector) {
    mFrustumRadar = new FrustumRadar();
}

/**
 * Updates the clipping frustum from the main camera.
 */
void ClippingJudge::update() {
    CameraDirector_RS* cameraDirector = mCameraDirector;
    FrustumRadar* frustumRadar = mFrustumRadar;
    const sead::LookAtCamera* camera;

    if (cameraDirector) {
        const sead::Matrix34f& viewMtx = getViewMtx_RS(this, 0);
        const sead::Matrix44f& projMtx = getProjectionMtx_RS(this, 0);
        frustumRadar->calcFrustumArea(viewMtx, projMtx, 300.0f, getFarClipping());
        camera = &getLookAtCamera(this, 0);
    } else {
        const sead::Matrix34f& viewMtx = getCameraViewMtx(this);
        const sead::Matrix44f& projMtx = getProjectionMtx(this);
        frustumRadar->calcFrustumArea(viewMtx, projMtx, 300.0f, getFarClipping());
        camera = &getLookAtCamera(mSceneCameraInfo, 0);
    }

    mCameraPos = camera->getPos();
}

/**
 * Updates the player position used for clipping.
 * @param pPlayerHolder player holder
 */
void ClippingJudge::setPlayerPos(const PlayerHolder* pPlayerHolder) {
    if (mIsUseClippingPosAsPlayerPos) {
        mPlayerPos = mCameraPos;
        return;
    }

    LiveActor* player = tryFindAlivePlayerActorFirst(pPlayerHolder);

    if (pPlayerHolder) {
        mPlayerPos = getTrans(player);
    }
}

/**
 * Gets the current far clip distance.
 * @return the far clip distance
 */
f32 ClippingJudge::getFarClipping() const {
    return mFarAreaObserver->mFarClipDistance * mFarAreaObserver->mFarClipRate;
}

/**
 * Gets the near clip distance of the frustum.
 * @return the near clip distance
 */
f32 ClippingJudge::getNearClipping() const {
    return mFrustumRadar->mNear;
}

/**
 * Checks whether a sphere is outside the frustum, ignoring far clip levels.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param near near clip distance
 * @return true if the sphere should be clipped
 */
bool ClippingJudge::isJudgedToClipFrustumUnUseFarLevel(const sead::Vector3f& rPos, f32 radius,
                                                       f32 near) const {
    return !mFrustumRadar->judgeInArea(rPos, radius, near);
}

/**
 * Checks whether a sphere is outside the frustum.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param near near clip distance
 * @return true if the sphere should be clipped
 */
bool ClippingJudge::judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near) const {
    return !mFrustumRadar->judgeInArea(rPos, radius, near);
}

/**
 * Checks whether a sphere is outside the frustum for a far clip level.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param near near clip distance
 * @param farLevel far clip level, 0 to ignore the far plane
 * @return true if the sphere should be clipped
 */
bool ClippingJudge::isJudgedToClipFrustum(const sead::Vector3f& rPos, f32 radius, f32 near,
                                          s32 farLevel) const {
    if (farLevel != 0) {
        return !mFrustumRadar->judgeInArea(rPos, radius, near);
    }

    return !mFrustumRadar->judgeInArea(rPos, radius, near, -1.0f);
}

/**
 * Checks whether a sphere is outside the frustum with a far distance.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param near near clip distance
 * @param far far clip distance
 * @return true if the sphere should be clipped
 */
bool ClippingJudge::judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near,
                                    f32 far) const {
    return !mFrustumRadar->judgeInArea(rPos, radius, near, far);
}

/**
 * Checks whether a point set is outside the frustum.
 * @param pPoints points to check
 * @param numPoints number of points
 * @param near near clip distance
 * @param farLevel far clip level
 * @return true if the points should be clipped
 */
bool ClippingJudge::isJudgedToClipFrustum(const sead::Vector3f* pPoints, s32 numPoints, f32 near,
                                          s32 farLevel) const {
    return !mFrustumRadar->judgeInArea(pPoints, numPoints, near, -1.0f);
}

/**
 * Checks whether a point set is outside the frustum with a far distance.
 * @param pPoints points to check
 * @param numPoints number of points
 * @param near near clip distance
 * @param far far clip distance
 * @return true if the points should be clipped
 */
bool ClippingJudge::judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near,
                                    f32 far) const {
    return !mFrustumRadar->judgeInArea(pPoints, numPoints, near, far);
}

/**
 * Checks whether a point set is outside the frustum.
 * @param pPoints points to check
 * @param numPoints number of points
 * @param near near clip distance
 * @return true if the points should be clipped
 */
bool ClippingJudge::judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints,
                                    f32 near) const {
    return !mFrustumRadar->judgeInArea(pPoints, numPoints, near, -1.0f);
}
}  // namespace al
