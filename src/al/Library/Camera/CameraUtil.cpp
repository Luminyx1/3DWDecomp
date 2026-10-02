#include "Library/Camera/CameraUtil.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Camera/CameraCreator.hpp"
#include "Library/Camera/CameraSwitcher.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Camera/CameraPoserAnim.hpp"
#include "Library/Play/Camera/CameraPoserKinopioBrigade.hpp"
#include "Library/Play/Camera/CameraPoserParallel.hpp"
#include "Library/Play/Camera/CameraPoserZoomParam.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/CameraPoser.hpp"
#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Camera/Info/SceneCameraControlInfo.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"
#include "Project/Camera/Param/CameraHolder.hpp"
#include "Project/Camera/Param/CameraInfo.hpp"

namespace al {

/**
 * Gets the look-at camera of the scene.
 * @param pInfo The scene camera info.
 * @return The look-at camera.
 */
const sead::LookAtCamera* getCameraLookAtCamera(const SceneCameraInfo* pInfo) {
    return pInfo->mLookAtCamera;
}

/**
 * Gets the projection of the scene camera.
 * @param pInfo The scene camera info.
 * @return The projection.
 */
const sead::PerspectiveProjection* getCameraProjection(const SceneCameraInfo* pInfo) {
    return static_cast<const sead::PerspectiveProjection*>(pInfo->mProjection);
}

/**
 * Gets the view matrix of the scene camera.
 * @param pInfo The scene camera info.
 * @return The view matrix.
 */
const sead::Matrix34f& getCameraViewMtx(const SceneCameraInfo* pInfo) {
    return *pInfo->mViewMtx;
}

/**
 * Gets the position of the scene camera.
 * @param pInfo The scene camera info.
 * @return The camera position.
 */
const sead::Vector3f& getCameraPos(const SceneCameraInfo* pInfo) {
    return getCameraLookAtCamera(pInfo)->getPos();
}

/**
 * Gets the look-at position of the scene camera.
 * @param pInfo The scene camera info.
 * @return The look-at position.
 */
const sead::Vector3f& getCameraLookAt(const SceneCameraInfo* pInfo) {
    return getCameraLookAtCamera(pInfo)->getAt();
}

/**
 * Gets the up direction of the scene camera.
 * @param pInfo The scene camera info.
 * @return The up direction.
 */
const sead::Vector3f& getCameraUpDir(const SceneCameraInfo* pInfo) {
    return getCameraLookAtCamera(pInfo)->getUp();
}

/**
 * Gets the near clipping distance of the scene camera.
 * @param pInfo The scene camera info.
 * @return The near clipping distance.
 */
f32 getCameraNear(const SceneCameraInfo* pInfo) {
    return getCameraProjection(pInfo)->getNear();
}

/**
 * Gets the far clipping distance of the scene camera.
 * @param pInfo The scene camera info.
 * @return The far clipping distance.
 */
f32 getCameraFar(const SceneCameraInfo* pInfo) {
    return getCameraProjection(pInfo)->getFar();
}

namespace {

/**
 * Gets the scene camera control info of a camera user.
 * @param pCamera Camera user.
 * @return Scene camera control info.
 */
SceneCameraControlInfo* getControlInfo(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mControlInfo;
}

/**
 * Gets the poser currently driving the scene camera.
 * @param pCamera Camera user.
 * @return Current camera poser.
 */
CameraPoser* getCurrentPoser(const IUseCamera* pCamera) {
    return getControlInfo(pCamera)->mCameraSwitcher->getCurrentPoser();
}

/**
 * Gets the poser of a camera info as an animation poser.
 * @param pInfo Camera info.
 * @return Animation poser, or nullptr if the poser is not an animation camera.
 */
CameraPoserAnim* tryGetAnimPoser(const CameraInfo* pInfo) {
    if (!isEqualString("Anim", pInfo->mPoser->mName)) {
        return nullptr;
    }

    return static_cast<CameraPoserAnim*>(pInfo->mPoser);
}

/**
 * Gets the current poser as a zoom poser.
 * @param pCamera Camera user.
 * @return Zoom poser, or nullptr if the current poser is not a zoom camera.
 */
CameraPoserZoom* tryGetZoomPoser(const IUseCamera* pCamera) {
    CameraPoser* poser = getCurrentPoser(pCamera);
    return isEqualString("Zoom", poser->mName) ? static_cast<CameraPoserZoom*>(poser) : nullptr;
}

/**
 * Storage for a placement id constructed later in place.
 */
union PlacementIdBuffer {
    PlacementIdBuffer() {}

    PlacementId id;
};

}  // namespace

/**
 * Gets the scene camera info of a camera user.
 * @param pCamera Camera user.
 * @return Scene camera info.
 */
SceneCameraInfo* getSceneCameraInfo(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo();
}

/**
 * Gets the main look-at camera.
 * @param pCamera Camera user.
 * @return Main look-at camera.
 */
const sead::LookAtCamera* getCameraLookAtCamera(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mLookAtCamera;
}

/**
 * Gets the main projection.
 * @param pCamera Camera user.
 * @return Main projection.
 */
const sead::PerspectiveProjection* getCameraProjection(const IUseCamera* pCamera) {
    return static_cast<const sead::PerspectiveProjection*>(
        pCamera->getSceneCameraInfo()->mProjection);
}

/**
 * Gets the sub look-at camera.
 * @param pCamera Camera user.
 * @return Sub look-at camera.
 */
const sead::LookAtCamera* getCameraLookAtCameraSub(const IUseCamera* pCamera) {
    return static_cast<const sead::LookAtCamera*>(pCamera->getSceneCameraInfo()->_28);
}

/**
 * Gets the sub projection.
 * @param pCamera Camera user.
 * @return Sub projection.
 */
const sead::PerspectiveProjection* getCameraProjectionSub(const IUseCamera* pCamera) {
    return static_cast<const sead::PerspectiveProjection*>(pCamera->getSceneCameraInfo()->_38);
}

/**
 * Gets the device projection matrix of the main projection.
 * @param pCamera Camera user.
 * @return Projection matrix.
 */
const sead::Matrix44f& getProjectionMtx(const IUseCamera* pCamera) {
    return getCameraProjection(pCamera)->getDeviceProjectionMatrix();
}

/**
 * Gets the view matrix of the main camera.
 * @param pCamera Camera user.
 * @return View matrix.
 */
const sead::Matrix34f& getCameraViewMtx(const IUseCamera* pCamera) {
    return getCameraLookAtCamera(pCamera)->getMatrix();
}

/**
 * Gets the device projection matrix of the sub projection.
 * @param pCamera Camera user.
 * @return Projection matrix.
 */
const sead::Matrix44f& getProjectionMtxSub(const IUseCamera* pCamera) {
    return getCameraProjectionSub(pCamera)->getDeviceProjectionMatrix();
}

/**
 * Gets the view matrix of the sub camera.
 * @param pCamera Camera user.
 * @return View matrix.
 */
const sead::Matrix34f& getCameraViewMtxSub(const IUseCamera* pCamera) {
    return getCameraLookAtCameraSub(pCamera)->getMatrix();
}

/**
 * Gets the position of the main camera.
 * @param pCamera Camera user.
 * @return Camera position.
 */
const sead::Vector3f& getCameraPos(const IUseCamera* pCamera) {
    return getCameraLookAtCamera(pCamera)->getPos();
}

/**
 * Gets the look-at position of the main camera.
 * @param pCamera Camera user.
 * @return Look-at position.
 */
const sead::Vector3f& getCameraLookAt(const IUseCamera* pCamera) {
    return getCameraLookAtCamera(pCamera)->getAt();
}

/**
 * Gets the up direction of the main camera.
 * @param pCamera Camera user.
 * @return Up direction.
 */
const sead::Vector3f& getCameraUpDir(const IUseCamera* pCamera) {
    return getCameraLookAtCamera(pCamera)->getUp();
}

/**
 * Gets the position of the sub camera.
 * @param pCamera Camera user.
 * @return Camera position.
 */
const sead::Vector3f& getCameraPosSub(const IUseCamera* pCamera) {
    return getCameraLookAtCameraSub(pCamera)->getPos();
}

/**
 * Gets the look-at position of the sub camera.
 * @param pCamera Camera user.
 * @return Look-at position.
 */
const sead::Vector3f& getCameraLookAtSub(const IUseCamera* pCamera) {
    return getCameraLookAtCameraSub(pCamera)->getAt();
}

/**
 * Gets the up direction of the sub camera.
 * @param pCamera Camera user.
 * @return Up direction.
 */
const sead::Vector3f& getCameraUpDirSub(const IUseCamera* pCamera) {
    return getCameraLookAtCameraSub(pCamera)->getUp();
}

/**
 * Gets the vertical field of view of the main projection.
 * @param pCamera Camera user.
 * @return Field of view in radians.
 */
f32 getCameraFovyRadian(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mProjection->getFovy();
}

/**
 * Gets the vertical field of view of the main projection.
 * @param pCamera Camera user.
 * @return Field of view in degrees.
 */
f32 getCameraFovyDegree(const IUseCamera* pCamera) {
    return sead::Mathf::rad2deg(getCameraFovyRadian(pCamera));
}

/**
 * Gets the near clip distance of the main projection.
 * @param pCamera Camera user.
 * @return Near clip distance.
 */
f32 getCameraNear(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mProjection->getNear();
}

/**
 * Gets the far clip distance of the main projection.
 * @param pCamera Camera user.
 * @return Far clip distance.
 */
f32 getCameraFar(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mProjection->getFar();
}

/**
 * Calculates how a snapshot image is rotated by the camera roll.
 * @param rCamera Camera the snapshot is taken with.
 * @param threshold Roll angle in degrees beyond which the image counts as rotated.
 * @return 0 if upright, 1 if rolled past -threshold, 3 if rolled past threshold.
 */
s32 calcSnapShotImageOrientation(const sead::LookAtCamera& rCamera, f32 threshold) {
    sead::Vector3f dir = rCamera.getAt() - rCamera.getPos();

    if (!tryNormalizeOrZero(&dir)) {
        return 0;
    }

    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f cameraUp = rCamera.getUp();
    verticalizeVec(&up, dir, up);
    verticalizeVec(&cameraUp, dir, cameraUp);

    if (!tryNormalizeOrZero(&up)) {
        return 0;
    }

    if (!tryNormalizeOrZero(&cameraUp)) {
        return 0;
    }

    f32 angle = calcAngleOnPlaneDegree(up, cameraUp, dir);

    if (angle > threshold) {
        return 3;
    }

    if (angle < -threshold) {
        return 1;
    }

    return 0;
}

/**
 * Gets the minimum distance of the current camera poser.
 * @param pCamera Camera user.
 * @return Minimum distance.
 */
f32 getCameraDistanceMin(const IUseCamera* pCamera) {
    return getCurrentPoser(pCamera)->getDistanceMin();
}

/**
 * Gets the maximum distance of the current camera poser.
 * @param pCamera Camera user.
 * @return Maximum distance.
 */
f32 getCameraDistanceMax(const IUseCamera* pCamera) {
    return getCurrentPoser(pCamera)->getDistanceMax();
}

/**
 * Builds the placement id of a camera owned by an actor.
 * @param pId Placement id to construct.
 * @param rInfo Init info of the owning actor.
 * @param pName Camera name, appended to the placement id if given.
 */
static void initCameraPlacementId(PlacementId* pId, const ActorInitInfo& rInfo,
                                  const char* pName) {
    const char* id = nullptr;
    const char* layerConfigName = nullptr;
    const char* unitConfigName = nullptr;
    const char* zoneId = nullptr;
    rInfo.getPlacementInfo().getPlacementIter().tryGetStringByKey(&id, "Id");
    rInfo.getPlacementInfo().getPlacementIter().tryGetStringByKey(&layerConfigName,
                                                                   "LayerConfigName");
    rInfo.getPlacementInfo().getZoneIter().tryGetStringByKey(&unitConfigName, "UnitConfigName");
    rInfo.getPlacementInfo().getZoneIter().tryGetStringByKey(&zoneId, "Id");

    const char* cameraId;

    if (id != nullptr) {
        if (pName != nullptr) {
            sead::FixedSafeString<64>* str = new sead::FixedSafeString<64>(id);
            str->appendWithFormat("_%s", pName);
            cameraId = str->cstr();
        } else {
            cameraId = id;
        }
    } else if (pName == nullptr) {
        cameraId = nullptr;
    } else if (isInStack(pName)) {
        cameraId = (new sead::FixedSafeString<64>(pName))->cstr();
    } else {
        cameraId = pName;
    }

    new (pId) PlacementId(cameraId, layerConfigName, unitConfigName, zoneId);
}

/**
 * Creates an object camera for an actor and registers it.
 * @param pCamera Camera user.
 * @param rInfo Init info of the owning actor.
 * @param pName Camera name.
 * @return Registered camera info.
 */
CameraInfo* initObjectCamera(const IUseCamera* pCamera, const ActorInitInfo& rInfo,
                             const char* pName) {
    PlacementIdBuffer idBuffer;
    PlacementId& id = idBuffer.id;
    initCameraPlacementId(&id, rInfo, pName);
    CameraPoser* poser = getControlInfo(pCamera)->mCameraCreator->createObjectCamera(id);
    sead::Matrix34f zoneMtx;

    if (tryGetZoneMatrixTR(&zoneMtx, rInfo.getPlacementInfo())) {
        poser->setZoneMatrix(zoneMtx);
    }

    getControlInfo(pCamera)->mCameraHolder->setCameraInfos(&id, poser, 3);
    return getControlInfo(pCamera)->mCameraHolder->getCameraInfoById(&id);
}

/**
 * Creates a programable camera for an actor and registers it.
 * @param pCamera Camera user.
 * @param rInfo Init info of the owning actor.
 * @param pName Camera name.
 * @return Registered camera info.
 */
CameraInfo* initProgramableCamera(const IUseCamera* pCamera, const ActorInitInfo& rInfo,
                                  const char* pName) {
    PlacementIdBuffer idBuffer;
    PlacementId& id = idBuffer.id;
    initCameraPlacementId(&id, rInfo, pName);
    CameraPoser* poser = getControlInfo(pCamera)->mCameraCreator->createProgramableCamera(id);
    getControlInfo(pCamera)->mCameraHolder->setCameraInfos(&id, poser, 3);
    return getControlInfo(pCamera)->mCameraHolder->getCameraInfoById(&id);
}

/**
 * Creates a programable camera without placement and registers it.
 * @param pCamera Camera user.
 * @param pName Camera name, used as its placement id.
 * @return Registered camera info.
 */
CameraInfo* initProgramableCamera(const IUseCamera* pCamera, const char* pName) {
    PlacementId id(pName, nullptr, nullptr, nullptr);
    CameraPoser* poser = getControlInfo(pCamera)->mCameraCreator->createProgramableCamera(id);
    getControlInfo(pCamera)->mCameraHolder->setCameraInfos(&id, poser, 3);
    return getControlInfo(pCamera)->mCameraHolder->getCameraInfoById(&id);
}

/**
 * Creates an animation camera for an actor and registers it.
 * @param pCamera Camera user.
 * @param rInfo Init info of the owning actor.
 * @param pResource Resource holding the camera animations.
 * @param pName Camera name.
 * @param isApplyAnimFovyAndTwist Whether the animation also drives the fovy and twist.
 * @return Registered camera info.
 */
CameraInfo* initAnimCamera(const IUseCamera* pCamera, const ActorInitInfo& rInfo,
                           const Resource* pResource, const char* pName,
                           bool isApplyAnimFovyAndTwist) {
    PlacementIdBuffer idBuffer;
    PlacementId& id = idBuffer.id;
    initCameraPlacementId(&id, rInfo, pName);
    SceneCameraInfo* sceneCameraInfo = pCamera->getSceneCameraInfo();
    CameraPoserAnim* poser = sceneCameraInfo->mControlInfo->mCameraCreator->createAnimCamera();
    sceneCameraInfo->mControlInfo->mCameraHolder->setCameraInfos(&id, poser, 3);
    poser->mIsApplyAnimFovyAndTwist = isApplyAnimFovyAndTwist ? true : false;
    poser->initAnim(pResource, id, rInfo);
    return sceneCameraInfo->mControlInfo->mCameraHolder->getCameraInfoById(&id);
}

/**
 * Creates the animation camera of an actor from its animation resource and registers it.
 * @param pActor Owning actor.
 * @param rInfo Init info of the actor.
 * @return Registered camera info.
 */
CameraInfo* initAnimCamera(const LiveActor* pActor, const ActorInitInfo& rInfo) {
    return initAnimCamera(pActor, rInfo, getAnimResource(pActor), "Anim", false);
}

/**
 * Creates an object camera for a map object and registers it.
 * @param pCamera Camera user.
 * @param rInfo Init info of the owning actor.
 * @param pName Camera name.
 * @return Registered camera info.
 */
CameraInfo* initObjectMapCamera(const IUseCamera* pCamera, const ActorInitInfo& rInfo,
                                const char* pName) {
    PlacementIdBuffer idBuffer;
    PlacementId& id = idBuffer.id;
    initCameraPlacementId(&id, rInfo, pName);
    CameraPoser* poser = getControlInfo(pCamera)->mCameraCreator->createObjectCamera(id);
    sead::Matrix34f zoneMtx;

    if (tryGetZoneMatrixTR(&zoneMtx, rInfo.getPlacementInfo())) {
        poser->setZoneMatrix(zoneMtx);
    }

    getControlInfo(pCamera)->mCameraHolder->setCameraInfos(&id, poser, 1);
    return getControlInfo(pCamera)->mCameraHolder->getCameraInfoById(&id);
}

/**
 * Starts a camera unless it is an animation camera.
 * @param pCamera Camera user.
 * @param pInfo Camera to start.
 * @param interpoleFrame Interpolation frames, 0 to cut without interpolation.
 */
void startCamera(const IUseCamera* pCamera, const CameraInfo* pInfo, s32 interpoleFrame) {
    if (isEqualString("Anim", pInfo->mPoser->mName)) {
        return;
    }

    if (interpoleFrame == 0) {
        requestCancelInterpole(pCamera);
    }

    getControlInfo(pCamera)->mCameraSwitcher->offLookAtStop();
    getControlInfo(pCamera)->mCameraSwitcher->start(*pInfo->mPlacementId, interpoleFrame);
}

/**
 * Requests the camera interpolation to be cancelled.
 * @param pCamera Camera user.
 */
void requestCancelInterpole(const IUseCamera* pCamera) {
    getControlInfo(pCamera)->mIsRequestCancelInterpole = true;
}

/**
 * Starts an animation camera with an animation.
 * @param pCamera Camera user.
 * @param pInfo Animation camera to start.
 * @param pAnimName Animation name.
 * @param pBaseMtx Base matrix the animation is relative to.
 * @param interpoleFrame Interpolation frames, 0 to cut without interpolation.
 */
void startAnimCamera(const IUseCamera* pCamera, const CameraInfo* pInfo, const char* pAnimName,
                     const sead::Matrix34f* pBaseMtx, s32 interpoleFrame) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return;
    }

    poser->setAnimAndBaseMtx(pAnimName, pBaseMtx);

    if (interpoleFrame == 0) {
        requestCancelInterpole(pCamera);
    }

    getControlInfo(pCamera)->mCameraSwitcher->start(*pInfo->mPlacementId, interpoleFrame);
}

/**
 * Starts an animation camera relative to an actor.
 * @param pActor Actor whose base matrix the animation is relative to.
 * @param pInfo Animation camera to start.
 * @param pAnimName Animation name.
 * @param interpoleFrame Interpolation frames, 0 to cut without interpolation.
 */
void startAnimCamera(const LiveActor* pActor, const CameraInfo* pInfo, const char* pAnimName,
                     s32 interpoleFrame) {
    startAnimCamera(pActor, pInfo, pAnimName, pActor->getBaseMtx(), interpoleFrame);
}

/**
 * Gets the frame count of an animation of an animation camera.
 * @param pInfo Animation camera.
 * @param pAnimName Animation name.
 * @return Frame count, or 0 if the camera is not an animation camera.
 */
s32 getCameraMaxFrame(const CameraInfo* pInfo, const char* pAnimName) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return 0;
    }

    return poser->getMaxFrame(pAnimName);
}

/**
 * Ends a camera.
 * @param pCamera Camera user.
 * @param pInfo Camera to end.
 * @param interpoleFrame Interpolation frames, 0 to cut without interpolation.
 */
void endCamera(const IUseCamera* pCamera, const CameraInfo* pInfo, s32 interpoleFrame) {
    if (interpoleFrame == 0) {
        requestCancelInterpole(pCamera);
    }

    getControlInfo(pCamera)->mCameraSwitcher->end(*pInfo->mPlacementId, interpoleFrame);
}

/**
 * Sets the player actor at the top of the screen.
 * @param pCamera Camera user.
 * @param pActor Top player actor.
 */
void setTopPlayerActor(const IUseCamera* pCamera, LiveActor* pActor) {
    getControlInfo(pCamera)->mTopPlayerActor = pActor;
}

/**
 * Sets the top player actor decided from the camera rail.
 * @param pCamera Camera user.
 * @param pActor Top player actor.
 */
void setTopPlayerActorFromRail(const IUseCamera* pCamera, LiveActor* pActor) {
    getControlInfo(pCamera)->mTopPlayerActorFromRail = pActor;
}

/**
 * Sets a player's position on the camera rail.
 * @param pActor Actor used to access the scene camera.
 * @param index Player index.
 * @param rPos Rail position.
 */
void setPlayerRailPos(const LiveActor* pActor, s32 index, const sead::Vector3f& rPos) {
    getControlInfo(pActor)->mPlayerRailPos[index] = rPos;
}

/**
 * Sets the player direction along the camera rail.
 * @param pActor Actor used to access the scene camera.
 * @param rDir Rail direction.
 */
void setPlayerRailDir(const LiveActor* pActor, const sead::Vector3f& rDir) {
    getControlInfo(pActor)->mPlayerRailDir = rDir;
}

/**
 * Gets the player direction along the camera rail.
 * @param pActor Actor used to access the scene camera.
 * @return Rail direction.
 */
const sead::Vector3f& getPlayerRailDir(const LiveActor* pActor) {
    return getControlInfo(pActor)->mPlayerRailDir;
}

/**
 * Requests the user camera control to be reset.
 * @param pCamera Camera user.
 */
void requestResetUserCameraControl(const IUseCamera* pCamera) {
    getControlInfo(pCamera)->mIsRequestResetUserControl = true;
}

/**
 * Requests a camera shake.
 * @param pCamera Camera user.
 * @param pShakeName Shake name.
 */
void requestStartCameraShake(const IUseCamera* pCamera, const char* pShakeName) {
    getControlInfo(pCamera)->mShakeName = pShakeName;
}

/**
 * Sets whether a player is a camera calculation target.
 * @param pActor Actor used to access the scene camera.
 * @param index Player index.
 * @param isTarget Whether the player is a target.
 */
void setCameraCalcTargetFlag(const LiveActor* pActor, s32 index, bool isTarget) {
    getControlInfo(pActor)->mIsCalcTarget[index] = isTarget;
}

/**
 * Sets whether a player actor is a camera calculation target.
 * @param pActor Player actor.
 * @param isTarget Whether the player is a target.
 */
void setCameraCalcTargetFlag(const LiveActor* pActor, bool isTarget) {
    s32 index = alPlayerFunction::findPlayerHolderIndex(pActor);
    getControlInfo(pActor)->mIsCalcTarget[index] = isTarget;
}

/**
 * Checks whether a player is a camera calculation target.
 * @param pActor Actor used to access the players.
 * @param index Player index.
 * @return Whether the player is alive and a target.
 */
bool isCameraCalcTarget(const LiveActor* pActor, s32 index) {
    if (isDead(getPlayerActor(pActor, index))) {
        return false;
    }

    return getControlInfo(pActor)->mIsCalcTarget[index];
}

/**
 * Checks whether a player actor is a camera calculation target.
 * @param pActor Player actor.
 * @return Whether the player is alive and a target.
 */
bool isCameraCalcTarget(const LiveActor* pActor) {
    if (isDead(pActor)) {
        return false;
    }

    s32 index = alPlayerFunction::findPlayerHolderIndex(pActor);
    return getControlInfo(pActor)->mIsCalcTarget[index];
}

/**
 * Disables user control of the current camera.
 * @param pActor Actor used to access the scene camera.
 */
void invalidUserCameraControl(const LiveActor* pActor) {
    getCurrentPoser(pActor)->_89 = false;
}

/**
 * Enables user control of the current camera.
 * @param pActor Actor used to access the scene camera.
 */
void validUserCameraControl(const LiveActor* pActor) {
    getCurrentPoser(pActor)->_89 = true;
}

/**
 * Gets the player actor at the top of the screen.
 * @param pCamera Camera user.
 * @return Top player actor.
 */
LiveActor* getTopPlayerActor(const IUseCamera* pCamera) {
    return getControlInfo(pCamera)->mTopPlayerActor;
}

/**
 * Gets a pointer to the scene view matrix.
 * @param pCamera Camera user.
 * @return View matrix pointer.
 */
const sead::Matrix34f* getCameraViewMtxPtr(const IUseCamera* pCamera) {
    return pCamera->getSceneCameraInfo()->mViewMtx;
}

/**
 * Sets the position of a camera.
 * @param pInfo Camera.
 * @param rPos Camera position.
 */
void setCameraPos(CameraInfo* pInfo, const sead::Vector3f& rPos) {
    pInfo->mPoser->setCameraPos(rPos);
}

/**
 * Sets the look-at position of a camera.
 * @param pInfo Camera.
 * @param rPos Look-at position.
 */
void setCameraLookAtPos(CameraInfo* pInfo, const sead::Vector3f& rPos) {
    pInfo->mPoser->setLookAtPos(rPos);
}

/**
 * Sets the up direction of a camera.
 * @param pInfo Camera.
 * @param rUp Up direction.
 */
void setCameraUpDir(CameraInfo* pInfo, const sead::Vector3f& rUp) {
    pInfo->mPoser->setUp(rUp);
}

/**
 * Sets the vertical field of view of a camera.
 * @param pInfo Camera.
 * @param fovy Field of view in degrees.
 */
void setCameraFovyDegree(CameraInfo* pInfo, f32 fovy) {
    pInfo->mPoser->setFovyDegree(fovy);
}

/**
 * Calculates the camera direction (from the look-at position to the camera).
 * @param pOut Output direction.
 * @param pCamera Camera user.
 */
void calcCameraDir(sead::Vector3f* pOut, const IUseCamera* pCamera) {
    const sead::Matrix34f* viewMtx = getCameraViewMtxPtr(pCamera);
    pOut->set(viewMtx->m[2][0], viewMtx->m[2][1], viewMtx->m[2][2]);
}

/**
 * Calculates the camera look direction.
 * @param pOut Output direction.
 * @param pCamera Camera user.
 */
void calcCameraLookDir(sead::Vector3f* pOut, const IUseCamera* pCamera) {
    calcCameraDir(pOut, pCamera);
    pOut->negate();
}

/**
 * Calculates the camera side direction.
 * @param pOut Output direction.
 * @param pCamera Camera user.
 */
void calcCameraSideDir(sead::Vector3f* pOut, const IUseCamera* pCamera) {
    const sead::Vector3f& up = getCameraUpDir(pCamera);
    sead::Vector3f lookDir;
    calcCameraLookDir(&lookDir, pCamera);
    pOut->setCross(lookDir, up);
    normalizeOrZero(pOut);
}

/**
 * Gets the interpolation frames of a camera.
 * @param pInfo Camera.
 * @return Interpolation frames.
 */
s32 getCameraInterpoleFrame(const CameraInfo* pInfo) {
    return pInfo->mPoser->mInterpolationFrame;
}

/**
 * Gets the poser name of a camera.
 * @param pInfo Camera.
 * @return Poser name.
 */
const char* getCameraPoserName(const CameraInfo* pInfo) {
    return pInfo->mPoser->mName;
}

/**
 * Checks the poser name of a camera.
 * @param pName Poser name to compare with.
 * @param pInfo Camera.
 * @return Whether the poser has the name.
 */
bool isEqualCameraPoserName(const char* pName, const CameraInfo* pInfo) {
    return isEqualString(pName, pInfo->mPoser->mName);
}

/**
 * Gets the current frame of an animation camera.
 * @param pInfo Animation camera.
 * @return Current frame, or -1 if the camera is not an animation camera.
 */
s32 getCurrentFrameAnimCamera(const CameraInfo* pInfo) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return -1;
    }

    return poser->mFrame;
}

/**
 * Gets the frame count of the current animation of an animation camera.
 * @param pInfo Animation camera.
 * @return Frame count, or -1 if the camera is not an animation camera.
 */
s32 getMaxFrameAnimCamera(const CameraInfo* pInfo) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return -1;
    }

    return poser->mMaxFrame;
}

/**
 * Checks whether the animation of an animation camera has ended.
 * @param pInfo Animation camera.
 * @return Whether the animation has ended.
 */
bool isEndAnimCamera(const CameraInfo* pInfo) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return false;
    }

    return poser->isEndAnim();
}

/**
 * Checks whether an animation camera has an animation.
 * @param pInfo Animation camera.
 * @param pAnimName Animation name.
 * @return Whether the animation exists.
 */
bool isExistAnimAnimCamera(const CameraInfo* pInfo, const char* pAnimName) {
    CameraPoserAnim* poser = tryGetAnimPoser(pInfo);

    if (poser == nullptr) {
        return false;
    }

    return poser->isExistAnim(pAnimName);
}

/**
 * Sets the look-at position pointer used for a player.
 * @param pCamera Camera user.
 * @param pActor Player actor.
 * @param pPos Look-at position pointer.
 */
void setCameraLookAtPosPtr(const IUseCamera* pCamera, LiveActor* pActor,
                           const sead::Vector3f* pPos) {
    s32 index = alPlayerFunction::findPlayerHolderIndex(pActor);
    getControlInfo(pCamera)->mLookAtPosPtrs[index] = pPos;
}

/**
 * Sets the look-at position pointer used for a player.
 * @param pCamera Camera user.
 * @param pSensor Sensor of the player actor.
 * @param pPos Look-at position pointer.
 */
void setCameraLookAtPosPtr(const IUseCamera* pCamera, const HitSensor* pSensor,
                           const sead::Vector3f* pPos) {
    s32 index = alPlayerFunction::findPlayerHolderIndex(pSensor);
    getControlInfo(pCamera)->mLookAtPosPtrs[index] = pPos;
}

/**
 * Sets the additional look-at position pointer placed after the players' ones.
 * @param pCamera Camera user.
 * @param pActor Actor used to access the players.
 * @param pPos Look-at position pointer.
 */
void setAdditionalCameraLookAtPosPtr(const IUseCamera* pCamera, LiveActor* pActor,
                                     const sead::Vector3f* pPos) {
    s32 index = getPlayerNumMax(pActor) + 1;
    getControlInfo(pCamera)->mLookAtPosPtrs[index] = pPos;
}

/**
 * Gets the camera rail holder.
 * @param pCamera Camera user.
 * @return Camera rail holder.
 */
CameraRailHolder* getCameraRailHolder(const IUseCamera* pCamera) {
    return getControlInfo(pCamera)->mCameraRailHolder;
}

/**
 * Disables user control of the current zoom camera.
 * @param pCamera Camera user.
 */
void invalidZoomCameraControl(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        poser->invalidControl();
    }
}

/**
 * Enables user control of the current zoom camera.
 * @param pCamera Camera user.
 */
void validZoomCameraControl(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        poser->validControl();
    }
}

/**
 * Requests the zoom level of the current zoom camera to be reset.
 * @param pCamera Camera user.
 */
void requestResetZoomCameraControl(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        poser->resetZoomLevel();
    }
}

/**
 * Lets only one controller port control the current zoom camera.
 * @param pCamera Camera user.
 * @param port Controller port.
 */
void validZoomCameraControlByPort(const IUseCamera* pCamera, s32 port) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        poser->setStickPlayerPort(port);
    }
}

/**
 * Gets the near zoom parameter of the current zoom camera.
 * @param pCamera Camera user.
 * @return Near zoom parameter, or the default one if the camera is not a zoom camera.
 */
const CameraPoserZoomParam& getZoomParamNear(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        return poser->getParamNear();
    }

    return CameraPoserZoom::getDefaultParamNear();
}

/**
 * Gets the normal zoom parameter of the current zoom camera.
 * @param pCamera Camera user.
 * @return Normal zoom parameter, or the default one if the camera is not a zoom camera.
 */
const CameraPoserZoomParam& getZoomParamNormal(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        return poser->getParamNormal();
    }

    return CameraPoserZoom::getDefaultParamNormal();
}

/**
 * Gets the far zoom parameter of the current zoom camera.
 * @param pCamera Camera user.
 * @return Far zoom parameter, or the default one if the camera is not a zoom camera.
 */
const CameraPoserZoomParam& getZoomParamFar(const IUseCamera* pCamera) {
    CameraPoserZoom* poser = tryGetZoomPoser(pCamera);

    if (poser != nullptr) {
        return poser->getParamFar();
    }

    return CameraPoserZoom::getDefaultParamFar();
}

/**
 * Makes the current Captain Toad camera ignore input for a while.
 * @param pCamera Camera user.
 * @param frame Number of frames to ignore input for.
 */
void requestCancelInputKinopioBrigadeCamera(const IUseCamera* pCamera, s32 frame) {
    if (isEqualString(getCurrentPoser(pCamera)->mName, "KinopioBrigade")) {
        static_cast<CameraPoserKinopioBrigade*>(getCurrentPoser(pCamera))->mWaitFrame = frame;
    }
}

/**
 * Switches a parallel camera to single player mode.
 * @param pInfo Camera.
 * @return Whether the camera is a parallel camera.
 */
bool tryChangeSingleCameraMode(const CameraInfo* pInfo) {
    if (!isEqualString(pInfo->mPoser->mName, "Parallel")) {
        return false;
    }

    static_cast<CameraPoserParallel*>(pInfo->mPoser)->changeSingleCameraMode();
    return true;
}

/**
 * Requests the gyro mode to be turned off.
 * @param pCamera Camera user.
 */
void requestOffGyroMode(const IUseCamera* pCamera) {
    getControlInfo(pCamera)->mCameraSwitcher->mIsRequestOffGyroMode = true;
}

/**
 * Checks whether the current camera is a parallel camera in superb view mode.
 * @param pCamera Camera user.
 * @return Whether the camera is in superb view mode.
 */
bool isInSuperbArea(const IUseCamera* pCamera) {
    CameraPoser* poser = getCurrentPoser(pCamera);

    if (isEqualString(poser->mName, "Parallel") &&
        static_cast<CameraPoserParallel*>(poser)->mIsForceGyroMode) {
        return true;
    }

    return false;
}

/**
 * Checks whether a camera is the current camera.
 * @param pCamera Camera user.
 * @param pInfo Camera.
 * @return Whether the camera is current.
 */
bool isActiveCamera(const IUseCamera* pCamera, const CameraInfo* pInfo) {
    if (pInfo == nullptr) {
        return false;
    }

    CameraPoser* poser = getCurrentPoser(pCamera);

    if (poser != nullptr && poser->mPlacementId->isEqual(*pInfo->mPlacementId)) {
        return true;
    }

    return false;
}

}  // namespace al

namespace alTempSeadUtility {

/**
 * Calculates the left and right eye cameras and projections of a stereo view.
 * @param pProjectionL Output projection of the left eye.
 * @param pCameraL Output camera of the left eye.
 * @param pProjectionR Output projection of the right eye.
 * @param pCameraR Output camera of the right eye.
 * @param rProjection Projection of the original view.
 * @param rCamera Camera of the original view.
 * @param depthLevel Distance to the plane that appears on the screen surface.
 * @param factor Strength of the stereo effect.
 * @param isRealSwitch Whether to emulate the real-world viewing distance.
 * @return Parallax of the base plane relative to the view width.
 */
f32 calcStereoCamera(sead::DirectProjection* pProjectionL, sead::DirectCamera* pCameraL,
                     sead::DirectProjection* pProjectionR, sead::DirectCamera* pCameraR,
                     const sead::Projection& rProjection, const sead::Camera& rCamera,
                     f32 depthLevel, f32 factor, bool isRealSwitch) {
    const sead::Matrix44f& projMtx = rProjection.getProjectionMatrix();
    f32 near = projMtx(2, 3) / projMtx(2, 2);
    f32 doubleNear = near + near;
    f32 top = near * (projMtx(1, 2) + 1.0f) / projMtx(1, 1);
    f32 far = projMtx(2, 2) * near / (projMtx(2, 2) + 1.0f);
    f32 right = near * (projMtx(0, 2) + 1.0f) / projMtx(0, 0);
    f32 left = right - doubleNear / projMtx(0, 0);
    f32 width = right - left;
    f32 bottom = top - doubleNear / projMtx(1, 1);
    f32 height = top - bottom;
    f32 depthRate = depthLevel / near;
    f32 baseHeight = depthRate * height;
    f32 screenRate = baseHeight / 46.08f;
    f32 distance;
    f32 newFar;
    f32 interval;

    if (isRealSwitch) {
        distance = screenRate * 289.0f;
        f32 newNear = distance - (depthLevel - near);
        newFar = (far - depthLevel) + distance;

        if (newNear <= 0.0f) {
            newNear = distance * 0.01f;
        }

        near = newNear;
        newFar = newFar <= near ? near + near : newFar;
        f32 nearRate = near / distance;
        f32 scale = baseHeight * nearRate / height;
        top *= scale;
        bottom *= scale;
        left *= scale;
        right *= scale;
        width = depthRate * width * nearRate;
        interval = screenRate * 62.0f;
    } else {
        newFar = far;

        if (far > depthLevel) {
            interval = screenRate * 5.99925f;
            interval = far / (far - depthLevel) * interval;
        } else {
            interval = 0.0f;
        }

        distance = depthLevel;
    }

    f32 halfInterval = interval * factor * 0.5f;
    f32 shift = (distance - near) / distance * halfInterval;
    right += shift;
    left -= shift;
    f32 rightR = right - halfInterval;
    f32 leftL = halfInterval + left;
    f32 leftR = rightR - width;
    f32 rightL = width + leftL;

    sead::FrustumProjection projection;
    projection.setTBLR(top, bottom, leftL, rightL);
    projection.setNear(near);
    projection.setFar(newFar);
    pProjectionL->setProjectionMatrix(projection.getProjectionMatrix(),
                                      sead::Graphics::cDevicePosture_Same);
    projection.setTBLR(top, bottom, leftR, rightR);
    projection.setNear(near);
    projection.setFar(newFar);
    pProjectionR->setProjectionMatrix(projection.getProjectionMatrix(),
                                      sead::Graphics::cDevicePosture_Same);

    sead::Vector3f pos;
    sead::Vector3f rightDir;
    sead::Vector3f upDir;
    sead::Vector3f lookDir;
    rCamera.getWorldPosByMatrix(&pos);
    rCamera.getRightVectorByMatrix(&rightDir);
    rCamera.getUpVectorByMatrix(&upDir);
    rCamera.getLookVectorByMatrix(&lookDir);
    f32 offset = distance - depthLevel;
    pos.x += offset * lookDir.x;
    pos.y += offset * lookDir.y;
    pos.z += offset * lookDir.z;

    {
        sead::LookAtCamera camera;
        sead::Vector3f eyePos = pos - rightDir * halfInterval;
        camera.setPos(eyePos);
        camera.setAt(eyePos - lookDir);
        camera.setUp(upDir);
        camera.normalizeUp();
        camera.updateViewMatrix();
        pCameraL->setDirectMatrix(camera.getMatrix());
        pCameraL->updateViewMatrix();
    }

    {
        sead::LookAtCamera camera;
        sead::Vector3f eyePos = pos + rightDir * halfInterval;
        camera.setPos(eyePos);
        camera.setAt(eyePos - lookDir);
        camera.setUp(upDir);
        camera.normalizeUp();
        camera.updateViewMatrix();
        pCameraR->setDirectMatrix(camera.getMatrix());
        pCameraR->updateViewMatrix();
    }

    return halfInterval / (width * (distance / near));
}

}  // namespace alTempSeadUtility
