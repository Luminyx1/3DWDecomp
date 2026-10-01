#include "Library/Scene/Scene.hpp"

#include "Library/Audio/AudioDirector.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Camera/CameraDirector.hpp"
#include "Library/Camera/CameraPoserSceneInfo_RS.hpp"
#include "Library/Draw/GraphicsInitArg.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Layout/LayoutKit.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Scene/SceneObjHolder.hpp"
#include "Library/Scene/SceneStopCtrl.hpp"
#include "Library/Scene/SceneUtil.hpp"
#include "Library/Screen/ScreenCoverCtrl.hpp"
#include "Library/Sequence/DemoDirector.hpp"
#include "Library/Stage/StageResourceKeeper.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioSystem.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Project/Scene/SceneInitInfo.hpp"

namespace al {
namespace {
inline AudioSystemInfo* getAudioSystemInfo(const SceneInitInfo& rInfo) {
    return static_cast<AudioSystem*>(rInfo.mGameSystemInfo->_0)->getAudioSystemInfo();
}
}  // namespace

/**
 * Constructs a scene.
 * @param pName scene name
 */
Scene::Scene(const char* pName) : NerveExecutor(pName), mName(pName) {
    mCameraPoserSceneInfo = new CameraPoserSceneInfo_RS();
}

/**
 * Destroys the scene and its kits.
 */
Scene::~Scene() {
    stopPadRumble(this);

    if (mAudioDirector != nullptr) {
        mAudioDirector->finalize();
    }

    delete mStageResourceKeeper;
    delete mLiveActorKit;
    delete mLayoutKit;
}

/**
 * Makes the scene alive.
 */
void Scene::appear() {
    mIsAlive = true;
}

/**
 * Kills the scene.
 */
void Scene::kill() {
    mIsAlive = false;
}

/**
 * Runs one frame of the scene.
 */
void Scene::movement() {
    if (!mIsExecute) {
        stall();
        return;
    }

    if (mSceneStopCtrl != nullptr) {
        mSceneStopCtrl->update();
    }

    if (mScreenCoverCtrl != nullptr) {
        mScreenCoverCtrl->update();
    }

    updateNerve();
    control();

    if (mAudioKeeper != nullptr) {
        mAudioKeeper->update();
    }

    if (mAudioDirector != nullptr) {
        mAudioDirector->update();
    }
}

/**
 * Draws the main screen.
 */
void Scene::drawMain() const {
    drawMain_();
}

/**
 * Draws the sub screen.
 */
void Scene::drawSub() const {
    drawSub_();
}

/**
 * Gets the scene camera info from the active camera director.
 * @return scene camera info
 */
SceneCameraInfo* Scene::getSceneCameraInfo() const {
    if (mIsUseCameraRS) {
        return mLiveActorKit->mCameraDirectorRS->getSceneCameraInfo();
    }

    return mLiveActorKit->mCameraDirector->getSceneCameraInfo();
}

/**
 * Gets the demo director of the live actor kit.
 * @return demo director
 */
DemoDirector* Scene::getDemoDirector() const {
    return mLiveActorKit->mDemoDirector;
}

/**
 * Sets the scene object holder.
 * @param pHolder scene object holder
 */
void Scene::initSceneObjHolder(SceneObjHolder* pHolder) {
    mSceneObjHolder = pHolder;
}

/**
 * Creates the stage resource keeper and loads the stage resources.
 * @param pStageName stage name
 * @param scenarioNo scenario number
 */
void Scene::initAndLoadStageResource(const char* pStageName, s32 scenarioNo) {
    mStageResourceKeeper = new StageResourceKeeper();
    mStageResourceKeeper->initAndLoadResource(pStageName, scenarioNo);
}

/**
 * Creates the live actor kit with default graphics.
 * @param rInfo scene init info
 * @param maxActors maximum actor count
 * @param maxPlayers maximum player count
 * @param maxCameras maximum camera count
 * @param maxViews maximum view count
 */
void Scene::initLiveActorKit(const SceneInitInfo& rInfo, s32 maxActors, s32 maxPlayers,
                             s32 maxCameras, s32 maxViews) {
    mLiveActorKit = new LiveActorKit(maxActors, maxPlayers, false, false);
    mLiveActorKit->mEffectSystem = rInfo.mGameSystemInfo->getEffectSystem();
    mLiveActorKit->init(maxCameras, maxViews, false);
    initPadRumble(this, rInfo);

    GraphicsInitArg arg;
    arg.mViewRendererCreator = new ViewRendererCreator();
    arg._20 = maxCameras;
    arg._14 = maxCameras;
    mLiveActorKit->initGraphics(arg, rInfo.mStageName);
}

/**
 * Creates the live actor kit without graphics.
 * @param rInfo scene init info
 * @param maxActors maximum actor count
 * @param maxPlayers maximum player count
 * @param maxCameras maximum camera count
 * @param maxViews maximum view count
 * @param isUseCameraRS whether the RS camera director is used
 * @param isUnk unknown flag
 */
void Scene::initLiveActorKitImpl(const SceneInitInfo& rInfo, s32 maxActors, s32 maxPlayers,
                                 s32 maxCameras, s32 maxViews, bool isUseCameraRS, bool isUnk) {
    mLiveActorKit = new LiveActorKit(maxActors, maxPlayers, isUseCameraRS, isUnk);
    mLiveActorKit->mEffectSystem = rInfo.mGameSystemInfo->getEffectSystem();
    mLiveActorKit->init(maxCameras, maxViews, isUseCameraRS);
    initPadRumble(this, rInfo);
}

/**
 * Sets the draw system info from the game system info.
 * @param rInfo scene init info
 */
void Scene::initDrawSystemInfo(const SceneInitInfo& rInfo) {
    mDrawSystemInfo = rInfo.mGameSystemInfo->getDrawSystemInfo();
}

/**
 * Creates the live actor kit and its graphics.
 * @param rArg graphics init arg
 * @param rInfo scene init info
 * @param maxActors maximum actor count
 * @param maxPlayers maximum player count
 * @param maxCameras maximum camera count
 * @param maxViews maximum view count
 * @param isUseCameraRS whether the RS camera director is used
 * @param isUnk unknown flag
 */
void Scene::initLiveActorKitWithGraphics(const GraphicsInitArg& rArg, const SceneInitInfo& rInfo,
                                         s32 maxActors, s32 maxPlayers, s32 maxCameras,
                                         s32 maxViews, bool isUseCameraRS, bool isUnk) {
    mLiveActorKit = new LiveActorKit(maxActors, maxPlayers, isUseCameraRS, isUnk);
    mLiveActorKit->mEffectSystem = rInfo.mGameSystemInfo->getEffectSystem();
    mLiveActorKit->init(maxCameras, maxViews, isUseCameraRS);
    initPadRumble(this, rInfo);
    mLiveActorKit->initGraphics(rArg, rInfo.mStageName);

    if (rArg._e) {
        GraphicsSystemInfo* graphicsInfo = mLiveActorKit->mGraphicsSystemInfo;

        if (graphicsInfo != nullptr) {
            u8* unk = *reinterpret_cast<u8**>(&graphicsInfo->_130[0x240 - 0x130]);

            if (unk && isUseCameraRS) {
                unk[0xe72] = true;
            }
        }
    }
}

/**
 * Creates the layout kit.
 * @param rInfo scene init info
 */
void Scene::initLayoutKit(const SceneInitInfo& rInfo) {
    mLayoutKit = new LayoutKit(rInfo.mGameSystemInfo->getFontHolder());
    mLayoutKit->setEffectSystem(rInfo.mGameSystemInfo->getEffectSystem());
    mLayoutKit->setLayoutSystem(rInfo.mGameSystemInfo->getLayoutSystem());
    mLayoutKit->setDrawContext(
        reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext));
}

/**
 * Creates the scene stop controller.
 */
void Scene::initSceneStopCtrl() {
    mSceneStopCtrl = new SceneStopCtrl();
}

/**
 * Creates the audio director of the scene.
 * @param rInfo scene init info
 * @param pStageName stage name
 * @param seRequestNum sound effect request count
 * @param unused1 unused
 * @param unused2 unused
 * @param isUseSituation unused
 * @param pBgmStageName bgm stage name
 * @param unused3 unused
 * @param volume volume
 */
void Scene::initSceneAudio(const SceneInitInfo& rInfo, const char* pStageName, s32 seRequestNum,
                           s32 unused1, s32 unused2, bool isUseSituation,
                           const char* pBgmStageName, s32 unused3, f32 volume) {
    AudioSystemInfo* audioSystemInfo = getAudioSystemInfo(rInfo);
    mAudioDirector = new AudioDirector();
    mAudioDirector->init(audioSystemInfo, pStageName, seRequestNum, unused1, unused2,
                         pBgmStageName, unused3, volume);

    SeadAudioPlayer* player = audioSystemInfo->getSeadAudioPlayerForSe();
    s32 playerNum = player->getSoundPlayerCount();
    const char* playerNames[20];

    for (s32 i = 0; i < playerNum; i++) {
        playerNames[i] = player->getSoundName(SeadAudioPlayer::getSoundPlayerIdFromIndex(i));
    }

    mAudioDirector->initSituationDirector(playerNames, playerNum);
}

/**
 * Initializes 3D audio of the scene.
 * @param rInfo scene init info
 * @param pCameraPos camera position
 * @param pCameraMtx camera matrix
 * @param pProjection camera projection
 * @param pCameraAt camera target
 * @param pStageName stage name
 * @param pAreaObjDirector area object director
 * @param isUseListenerPoser whether a listener poser is used
 */
void Scene::initSceneAudio3D(const SceneInitInfo& rInfo, const sead::Vector3<f32>* pCameraPos,
                             const sead::Matrix34<f32>* pCameraMtx,
                             const sead::PerspectiveProjection* pProjection,
                             const sead::Vector3<f32>* pCameraAt, const char* pStageName,
                             AreaObjDirector* pAreaObjDirector, bool isUseListenerPoser) {
    mAudioDirector->init3D(getAudioSystemInfo(rInfo), pCameraPos, pCameraMtx, pProjection,
                           pCameraAt, pStageName, pAreaObjDirector, isUseListenerPoser);
}

/**
 * Initializes the scene audio after placement.
 * @param rInfo scene init info
 */
void Scene::initSceneAudioAfterInitPlacement(const SceneInitInfo& rInfo) {
    mAudioDirector->initAfterInitPlacement(getAudioSystemInfo(rInfo));
}

/**
 * Creates the audio keeper of the scene.
 * @param pName audio keeper name
 */
void Scene::initAudioKeeper(const char* pName) {
    mAudioKeeper = createAudioKeeper(pName, mAudioDirector);
}

/**
 * Creates the screen cover controller.
 */
void Scene::initScreenCoverCtrl() {
    mScreenCoverCtrl = new ScreenCoverCtrl();
}

/**
 * Finishes scene initialization.
 * @param rInfo actor init info
 * @param pChecker scenario complete checker
 */
void Scene::endInit(const ActorInitInfo& rInfo, IScenarioCompleteChecker* pChecker) {
    if (mSceneObjHolder != nullptr) {
        mSceneObjHolder->initAfterPlacementSceneObj(rInfo);
    }

    if (mLiveActorKit != nullptr) {
        if (mLiveActorKit->mDemoDirector != nullptr) {
            mLiveActorKit->mDemoDirector->endInit(rInfo);
        }

        mLiveActorKit->endInit(pChecker);
    }

    if (mLayoutKit != nullptr) {
        mLayoutKit->endInit();
    }
}
}  // namespace al
