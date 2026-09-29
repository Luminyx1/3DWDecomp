#include "Project/Clipping/ClippingJudge.hpp"
#include <gfx/seadCamera.h>
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/Clipping/ClippingFarAreaObserver.hpp"
#include "Project/Clipping/FrustumRadar.hpp"

namespace al {
    class LiveActor;

    const sead::Matrix34f& getViewMtx_RS(const IUseCamera_RS* pUser, s32 index);
    const sead::Matrix44f& getProjectionMtx_RS(const IUseCamera_RS* pUser, s32 index);
    const sead::LookAtCamera& getLookAtCamera(const IUseCamera_RS* pUser, s32 index);
    const sead::Matrix34f& getCameraViewMtx(const IUseCamera* pUser);
    const sead::Matrix44f& getProjectionMtx(const IUseCamera* pUser);
    const sead::LookAtCamera& getLookAtCamera(const SceneCameraInfo* pInfo, s32 index);
    LiveActor* tryFindAlivePlayerActorFirst(const PlayerHolder* pHolder);

    /**
     * @brief Constructs the judge and its frustum radar.
     * @param pFarAreaObserver The observer providing the far clip distance.
     * @param pSceneCameraInfo The scene's camera info.
     * @param pCameraDirector The scene's camera director, or nullptr to use the scene camera info.
     */
    ClippingJudge::ClippingJudge(const ClippingFarAreaObserver* pFarAreaObserver, SceneCameraInfo* pSceneCameraInfo,
                                 CameraDirector_RS* pCameraDirector)
        : mFarAreaObserver(pFarAreaObserver), mFrustumRadar(nullptr), mSceneCameraInfo(pSceneCameraInfo),
          mCameraDirector(pCameraDirector), mIsUseClippingPosAsPlayerPos(false) {
        mFrustumRadar = new FrustumRadar();
    }

    /** @brief Updates the frustum and camera position from the current camera. */
    void ClippingJudge::update() {
        CameraDirector_RS* cameraDirector = mCameraDirector;
        FrustumRadar* radar = mFrustumRadar;
        const sead::LookAtCamera* lookAtCamera;

        if (cameraDirector != nullptr) {
            const sead::Matrix34f& viewMtx = getViewMtx_RS(this, 0);
            const sead::Matrix44f& projMtx = getProjectionMtx_RS(this, 0);
            radar->calcFrustumArea(viewMtx, projMtx, 300.0f, getFarClipping());
            lookAtCamera = &getLookAtCamera(this, 0);
        } else {
            const sead::Matrix34f& viewMtx = getCameraViewMtx(this);
            const sead::Matrix44f& projMtx = getProjectionMtx(this);
            radar->calcFrustumArea(viewMtx, projMtx, 300.0f, getFarClipping());
            lookAtCamera = &getLookAtCamera(mSceneCameraInfo, 0);
        }

        mCameraPos = lookAtCamera->getPos();
    }

    /**
     * @brief Updates the position used as the player's position for clipping.
     * @param pPlayerHolder The scene's player holder.
     */
    void ClippingJudge::setPlayerPos(const PlayerHolder* pPlayerHolder) {
        if (mIsUseClippingPosAsPlayerPos) {
            mPlayerPos = mCameraPos;
            return;
        }

        LiveActor* player = tryFindAlivePlayerActorFirst(pPlayerHolder);
        if (pPlayerHolder != nullptr) {
            mPlayerPos = getTrans(player);
        }
    }

    /**
     * @brief Gets the current far clip distance.
     * @return The far clip distance.
     */
    f32 ClippingJudge::getFarClipping() const {
        return mFarAreaObserver->getFarClipDistance();
    }

    /**
     * @brief Gets the current near clip distance.
     * @return The near clip distance.
     */
    f32 ClippingJudge::getNearClipping() const {
        return mFrustumRadar->getNear();
    }

    /**
     * @brief Checks whether a sphere is outside of the frustum.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @return Whether the sphere is clipped.
     */
    bool ClippingJudge::isJudgedToClipFrustumUnUseFarLevel(const sead::Vector3f& rPos, f32 radius, f32 near) const {
        return judgeInAreaCore(rPos, radius, near);
    }

    /**
     * @brief Checks whether a sphere is outside of the frustum.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @return Whether the sphere is clipped.
     */
    bool ClippingJudge::judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near) const {
        return !mFrustumRadar->judgeInArea(rPos, radius, near);
    }

    /**
     * @brief Checks whether a sphere is outside of the frustum.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @param farLevel The far clip level, 0 disables the far plane.
     * @return Whether the sphere is clipped.
     */
    bool ClippingJudge::isJudgedToClipFrustum(const sead::Vector3f& rPos, f32 radius, f32 near, s32 farLevel) const {
        if (farLevel == 0) {
            return judgeInAreaCore(rPos, radius, near, -1.0f);
        }

        return judgeInAreaCore(rPos, radius, near);
    }

    /**
     * @brief Checks whether a sphere is outside of the frustum.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @param far The far clip distance, or a non-positive value for no far plane.
     * @return Whether the sphere is clipped.
     */
    bool ClippingJudge::judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near, f32 far) const {
        return !mFrustumRadar->judgeInArea(rPos, radius, near, far);
    }

    /**
     * @brief Checks whether a set of points is outside of the frustum.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param near The near clip distance.
     * @param farLevel The far clip level (unused).
     * @return Whether the points are clipped.
     */
    bool ClippingJudge::isJudgedToClipFrustum(const sead::Vector3f* pPoints, s32 numPoints, f32 near, s32 farLevel) const {
        return judgeInAreaCore(pPoints, numPoints, near, -1.0f);
    }

    /**
     * @brief Checks whether a set of points is outside of the frustum.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param near The near clip distance.
     * @param far The far clip distance, or a negative value to use the radar's far clip distance.
     * @return Whether the points are clipped.
     */
    bool ClippingJudge::judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near, f32 far) const {
        return !mFrustumRadar->judgeInArea(pPoints, numPoints, near, far);
    }

    /**
     * @brief Checks whether a set of points is outside of the frustum, using the radar's far clip distance.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param near The near clip distance.
     * @return Whether the points are clipped.
     */
    bool ClippingJudge::judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near) const {
        return judgeInAreaCore(pPoints, numPoints, near, -1.0f);
    }
};
