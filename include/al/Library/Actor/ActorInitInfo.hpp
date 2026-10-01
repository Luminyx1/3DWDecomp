#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace al {
class AreaObjDirector;
class AudioDirector;
class CameraDirector_RS;
class ClippingDirectorBase;
class CollisionDirector;
class DemoDirector;
class EffectSystemInfo;
class ExecuteDirector;
class GraphicsSystemInfo;
class HitSensorDirector;
class ItemDirectorBase;
class LayoutInitInfo;
class LiveActor;
class LiveActorGroup;
class Nerve;
class OceanWaveDirector;
class PadRumbleDirector;
class PlacementId;
struct PlacementInfo;
class PlayerHolder;
class SceneCameraInfo;
class SceneObjHolder;
class SceneStopCtrl;
class ScreenCoverCtrl;
class ScreenPointDirector;
class ShadowDirector;
class StageSwitchDirector;

class ActorInitInfo {
public:
    ActorInitInfo();

    void initNew(const PlacementInfo* pPlacementInfo, const LayoutInitInfo* pLayoutInitInfo,
                 ExecuteDirector* pExecuteDirector, AudioDirector* pAudioDirector,
                 EffectSystemInfo* pEffectSystemInfo, OceanWaveDirector* pOceanWaveDirector,
                 SceneObjHolder* pSceneObjHolder, SceneStopCtrl* pSceneStopCtrl,
                 ScreenCoverCtrl* pScreenCoverCtrl, HitSensorDirector* pHitSensorDirector,
                 ScreenPointDirector* pScreenPointDirector,
                 ClippingDirectorBase* pClippingDirector, CollisionDirector* pCollisionDirector,
                 AreaObjDirector* pAreaObjDirector, StageSwitchDirector* pStageSwitchDirector,
                 PlayerHolder* pPlayerHolder, ItemDirectorBase* pItemDirector,
                 ShadowDirector* pShadowDirector, PadRumbleDirector* pPadRumbleDirector,
                 CameraDirector_RS* pCameraDirector, GraphicsSystemInfo* pGraphicsSystemInfo,
                 SceneCameraInfo* pSceneCameraInfo, DemoDirector* pDemoDirector,
                 LiveActorGroup* pLiveActorGroup, bool isUseCameraRS);

    void initViewIdSelf(const PlacementInfo *, const ActorInitInfo &);

    void initViewIdHostActor(const ActorInitInfo &, const LiveActor *);

    void copyHostInfo(const ActorInitInfo& rInfo, const PlacementInfo* pPlacementInfo);
    void initViewIdHost(const PlacementInfo* pPlacementInfo, const ActorInitInfo& rInfo);
    void initNoViewId(const PlacementInfo* pPlacementInfo, const ActorInitInfo& rInfo);

    const PlacementInfo& getPlacementInfo() const { return *mPlacementInfo; }
    const LayoutInitInfo* getLayoutInitInfo() const { return mLayoutInitInfo; }
    const ActorSceneInfo& getActorSceneInfo() const { return mActorSceneInfo; }
    ExecuteDirector* getExecuteDirector() const { return mExecuteDirector; }
    AudioDirector* getAudioDirector() const { return mAudioDirector; }
    EffectSystemInfo* getEffectSystemInfo() const { return mEffectSystemInfo; }
    OceanWaveDirector* getOceanWaveDirector() const { return mOceanWaveDirector; }
    HitSensorDirector* getHitSensorDirector() const { return mHitSensorDirector; }
    StageSwitchDirector* getStageSwitchDirector() const { return mStageSwitchDirector; }
    ScreenPointDirector* getScreenPointDirector() const { return mScreenPointerDirector; }
    PlacementId* getPlacementId() const { return mPlacementId; }
    LiveActorGroup* getLiveActorGroup() const { return mLiveActorGroup; }

    PlacementInfo* mPlacementInfo = nullptr;
    const LayoutInitInfo* mLayoutInitInfo = nullptr;
    ActorSceneInfo mActorSceneInfo;
    ExecuteDirector* mExecuteDirector = nullptr;
    AudioDirector* mAudioDirector = nullptr;
    EffectSystemInfo* mEffectSystemInfo = nullptr;
    OceanWaveDirector* mOceanWaveDirector = nullptr;
    HitSensorDirector* mHitSensorDirector = nullptr;
    StageSwitchDirector* mStageSwitchDirector = nullptr;
    ScreenPointDirector* mScreenPointerDirector = nullptr;
    PlacementId* mPlacementId;
    LiveActorGroup* mLiveActorGroup = nullptr;
};
}  // namespace al
