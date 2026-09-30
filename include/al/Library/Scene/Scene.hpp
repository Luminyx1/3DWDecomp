#pragma once

#include <prim/seadSafeString.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Camera/Core/IUseCameraDirector.hpp"

namespace sead {
template <typename T>
struct Vector3;
template <typename T>
class Matrix34;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class ActorInitInfo;
class AreaObjDirector;
class AudioDirector;
class AudioKeeper;
class CameraPoserSceneInfo_RS;
class DemoDirector;
struct DrawSystemInfo;
struct GraphicsInitArg;
class IScenarioCompleteChecker;
class LayoutKit;
class LiveActorKit;
class OceanWaveDirector;
struct PlacementInfo;
class SceneObjHolder;
class SceneStopCtrl;
class ScreenCoverCtrl;
class StageResourceKeeper;
struct SceneInitInfo;

class Scene : public NerveExecutor,
              public IUseAudioKeeper,
              public IUseCamera,
              public IUseSceneObjHolder {
public:
    Scene(const char* pName);
    ~Scene() override;

    virtual void init(const SceneInitInfo& rInfo) {}
    virtual void appear();
    virtual void kill();
    virtual void control() {}
    virtual void stall() {}

    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }
    SceneObjHolder* getSceneObjHolder() const override { return mSceneObjHolder; }
    SceneCameraInfo* getSceneCameraInfo() const override;

    virtual bool isValidPlacementParent(const PlacementInfo& rInfo) const { return true; }
    virtual bool isValidPlacement(const PlacementInfo& rInfo) const { return true; }
    virtual void drawMain_() const {}
    virtual void drawSub_() const {}

    void movement();
    void drawMain() const;
    void drawSub() const;
    DemoDirector* getDemoDirector() const;
    void initSceneObjHolder(SceneObjHolder* pHolder);
    void initAndLoadStageResource(const char* pStageName, s32 scenarioNo);
    void initLiveActorKit(const SceneInitInfo& rInfo, s32 maxActors, s32 maxPlayers,
                          s32 maxCameras, s32 maxViews);
    void initLiveActorKitImpl(const SceneInitInfo& rInfo, s32 maxActors, s32 maxPlayers,
                              s32 maxCameras, s32 maxViews, bool isUseCameraRS, bool isUnk);
    void initDrawSystemInfo(const SceneInitInfo& rInfo);
    void initLiveActorKitWithGraphics(const GraphicsInitArg& rArg, const SceneInitInfo& rInfo,
                                      s32 maxActors, s32 maxPlayers, s32 maxCameras,
                                      s32 maxViews, bool isUseCameraRS, bool isUnk);
    void initLayoutKit(const SceneInitInfo& rInfo);
    void initSceneStopCtrl();
    void initSceneAudio(const SceneInitInfo& rInfo, const char* pStageName, s32 seRequestNum,
                        s32 unused1, s32 unused2, bool isUseSituation, const char* pBgmStageName,
                        s32 unused3, f32 volume);
    void initSceneAudio3D(const SceneInitInfo& rInfo, const sead::Vector3<f32>* pCameraPos,
                          const sead::Matrix34<f32>* pCameraMtx,
                          const sead::PerspectiveProjection* pProjection,
                          const sead::Vector3<f32>* pCameraAt, const char* pStageName,
                          AreaObjDirector* pAreaObjDirector, bool isUseListenerPoser);
    void initSceneAudioAfterInitPlacement(const SceneInitInfo& rInfo);
    void initAudioKeeper(const char* pName);
    void initScreenCoverCtrl();
    void endInit(const ActorInitInfo& rInfo, IScenarioCompleteChecker* pChecker);

    const char* getName() const { return mName.cstr(); }
    bool isAlive() const { return mIsAlive; }
    StageResourceKeeper* getStageResourceKeeper() const { return mStageResourceKeeper; }
    LiveActorKit* getLiveActorKit() const { return mLiveActorKit; }
    LayoutKit* getLayoutKit() const { return mLayoutKit; }
    SceneStopCtrl* getSceneStopCtrl() const { return mSceneStopCtrl; }
    AudioDirector* getAudioDirector() const { return mAudioDirector; }
    ScreenCoverCtrl* getScreenCoverCtrl() const { return mScreenCoverCtrl; }
    DrawSystemInfo* getDrawSystemInfo() const { return mDrawSystemInfo; }
    CameraPoserSceneInfo_RS* getCameraPoserSceneInfo() const { return mCameraPoserSceneInfo; }
    OceanWaveDirector* getOceanWaveDirector() const { return mOceanWaveDirector; }
    bool isUseCameraRS() const { return mIsUseCameraRS; }

    sead::FixedSafeString<0x40> mName;
    bool mIsAlive = false;
    bool mIsExecute = true;
    StageResourceKeeper* mStageResourceKeeper = nullptr;
    LiveActorKit* mLiveActorKit = nullptr;
    LayoutKit* mLayoutKit = nullptr;
    SceneObjHolder* mSceneObjHolder = nullptr;
    SceneStopCtrl* mSceneStopCtrl = nullptr;
    AudioDirector* mAudioDirector = nullptr;
    AudioKeeper* mAudioKeeper = nullptr;
    ScreenCoverCtrl* mScreenCoverCtrl = nullptr;
    DrawSystemInfo* mDrawSystemInfo = nullptr;
    CameraPoserSceneInfo_RS* mCameraPoserSceneInfo = nullptr;
    OceanWaveDirector* mOceanWaveDirector = nullptr;
    bool mIsUseCameraRS = false;
};

static_assert(sizeof(Scene) == 0xe8);
}  // namespace al
