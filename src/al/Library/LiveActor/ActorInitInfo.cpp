#include "Library/Actor/ActorInitInfo.hpp"

#include "Library/Clipping/ClippingActorInfo.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
/**
 * Constructs empty actor init info with a new view id.
 */
ActorInitInfo::ActorInitInfo() : mPlacementId(new PlacementId()) {}

/**
 * Initializes actor init info with the given scene objects.
 * @param pPlacementInfo The placement info.
 * @param pLayoutInitInfo The layout init info.
 * @param pExecuteDirector The execute director.
 * @param pAudioDirector The audio director.
 * @param pEffectSystemInfo The effect system info.
 * @param pOceanWaveDirector The ocean wave director.
 * @param pSceneObjHolder The scene object holder.
 * @param pSceneStopCtrl The scene stop control.
 * @param pScreenCoverCtrl The screen cover control.
 * @param pHitSensorDirector The hit sensor director.
 * @param pScreenPointDirector The screen point director.
 * @param pClippingDirector The clipping director.
 * @param pCollisionDirector The collision director.
 * @param pAreaObjDirector The area object director.
 * @param pStageSwitchDirector The stage switch director.
 * @param pPlayerHolder The player holder.
 * @param pItemDirector The item director.
 * @param pShadowDirector The shadow director.
 * @param pPadRumbleDirector The pad rumble director.
 * @param pCameraDirector The camera director.
 * @param pGraphicsSystemInfo The graphics system info.
 * @param pSceneCameraInfo The scene camera info.
 * @param pDemoDirector The demo director.
 * @param pLiveActorGroup The group of all actors.
 * @param isSingleMode Whether the scene is in single player mode.
 */
void ActorInitInfo::initNew(
    const PlacementInfo* pPlacementInfo, const LayoutInitInfo* pLayoutInitInfo,
    ExecuteDirector* pExecuteDirector, AudioDirector* pAudioDirector,
    EffectSystemInfo* pEffectSystemInfo, OceanWaveDirector* pOceanWaveDirector,
    SceneObjHolder* pSceneObjHolder, SceneStopCtrl* pSceneStopCtrl,
    ScreenCoverCtrl* pScreenCoverCtrl, HitSensorDirector* pHitSensorDirector,
    ScreenPointDirector* pScreenPointDirector, ClippingDirectorBase* pClippingDirector,
    CollisionDirector* pCollisionDirector, AreaObjDirector* pAreaObjDirector,
    StageSwitchDirector* pStageSwitchDirector, PlayerHolder* pPlayerHolder,
    ItemDirectorBase* pItemDirector, ShadowDirector* pShadowDirector,
    PadRumbleDirector* pPadRumbleDirector, CameraDirector_RS* pCameraDirector,
    GraphicsSystemInfo* pGraphicsSystemInfo, SceneCameraInfo* pSceneCameraInfo,
    DemoDirector* pDemoDirector, LiveActorGroup* pLiveActorGroup, bool isSingleMode) {
    mLiveActorGroup = pLiveActorGroup;
    mPlacementInfo = const_cast<PlacementInfo*>(pPlacementInfo);
    mLayoutInitInfo = pLayoutInitInfo;
    mActorSceneInfo.cameraDirector = pCameraDirector;
    mActorSceneInfo._78 = pGraphicsSystemInfo;
    mExecuteDirector = pExecuteDirector;
    mActorSceneInfo.isSingleMode = isSingleMode;
    mAudioDirector = pAudioDirector;
    mEffectSystemInfo = pEffectSystemInfo;
    mOceanWaveDirector = pOceanWaveDirector;
    mHitSensorDirector = pHitSensorDirector;
    mStageSwitchDirector = pStageSwitchDirector;
    mScreenPointerDirector = pScreenPointDirector;
    mActorSceneInfo.sceneObjHolder = pSceneObjHolder;
    mActorSceneInfo.clippingDirectorBase = pClippingDirector;
    mActorSceneInfo.collisionDirector = pCollisionDirector;
    mActorSceneInfo.playerHolder = pPlayerHolder;
    mActorSceneInfo.sceneCameraInfo = pSceneCameraInfo;
    mActorSceneInfo.sceneStopCtrl = pSceneStopCtrl;
    mActorSceneInfo.screenCoverCtrl = pScreenCoverCtrl;
    mActorSceneInfo.itemDirectorBase = pItemDirector;
    mActorSceneInfo.demoDirector = pDemoDirector;
    mActorSceneInfo.areaObjDirector = pAreaObjDirector;
    mActorSceneInfo.shadowDirector = pShadowDirector;
    mActorSceneInfo.padRumbleDirector = pPadRumbleDirector;
    alPlacementFunction::getClippingViewId(mPlacementId, *pPlacementInfo);
}

/**
 * Takes the view id from a placement and copies the rest from host init info.
 * @param pPlacementInfo The placement info.
 * @param rInfo The host init info.
 */
void ActorInitInfo::initViewIdSelf(const PlacementInfo* pPlacementInfo, const ActorInitInfo& rInfo) {
    alPlacementFunction::getClippingViewId(mPlacementId, *pPlacementInfo);
    copyHostInfo(rInfo, pPlacementInfo);
}

/**
 * Copies the scene objects of host init info and sets the placement info.
 * @param rInfo The host init info.
 * @param pPlacementInfo The placement info.
 */
void ActorInitInfo::copyHostInfo(const ActorInitInfo& rInfo, const PlacementInfo* pPlacementInfo) {
    mPlacementInfo = const_cast<PlacementInfo*>(pPlacementInfo);
    mLayoutInitInfo = rInfo.mLayoutInitInfo;
    mActorSceneInfo = rInfo.mActorSceneInfo;
    mExecuteDirector = rInfo.mExecuteDirector;
    mEffectSystemInfo = rInfo.mEffectSystemInfo;
    mAudioDirector = rInfo.mAudioDirector;
    mOceanWaveDirector = rInfo.mOceanWaveDirector;
    mHitSensorDirector = rInfo.mHitSensorDirector;
    mScreenPointerDirector = rInfo.mScreenPointerDirector;
    mStageSwitchDirector = rInfo.mStageSwitchDirector;
    mLiveActorGroup = rInfo.mLiveActorGroup;
}

/**
 * Uses the view id of host init info and copies the rest from it.
 * @param pPlacementInfo The placement info.
 * @param rInfo The host init info.
 */
void ActorInitInfo::initViewIdHost(const PlacementInfo* pPlacementInfo, const ActorInitInfo& rInfo) {
    mPlacementId = rInfo.mPlacementId;
    copyHostInfo(rInfo, pPlacementInfo);
}

/**
 * Uses the view id of a host actor and copies the rest from host init info.
 * @param rInfo The host init info.
 * @param pActor The host actor.
 */
void ActorInitInfo::initViewIdHostActor(const ActorInitInfo& rInfo, const LiveActor* pActor) {
    const ClippingActorInfo* clippingInfo = static_cast<const ClippingActorInfo*>(
        pActor->getSceneInfo()->clippingDirectorBase->findActorInfo(pActor));
    mPlacementId = (clippingInfo == nullptr) ? rInfo.mPlacementId : clippingInfo->mPlacementId;
    copyHostInfo(rInfo, rInfo.mPlacementInfo);
}

/**
 * Clears the view id and copies the rest from host init info.
 * @param pPlacementInfo The placement info.
 * @param rInfo The host init info.
 */
void ActorInitInfo::initNoViewId(const PlacementInfo* pPlacementInfo, const ActorInitInfo& rInfo) {
    mPlacementId = nullptr;
    copyHostInfo(rInfo, pPlacementInfo);
}
}  // namespace al
