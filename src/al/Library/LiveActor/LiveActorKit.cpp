#include "Library/LiveActor/Common/LiveActorKit.hpp"

#include "Library/Collision/CollisionDirector.hpp"
#include "Library/Camera/CameraDirector.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Controller/PadRumbleDirector.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Draw/ViewRenderer.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/HitSensor/HitSensorDirector.hpp"
#include "Library/Item/ItemDirectorBase.hpp"
#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Player/PlayerHolder.hpp"
#include "Library/Screen/ScreenPointDirector.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/SwitchAreaDirector.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Project/Clipping/ClippingAreaDirector.hpp"
#include "Project/Clipping/ClippingDirector.hpp"
#include "Project/Execute/ExecuteSystemInitInfo.hpp"
#include "Project/Framework/MultiCoreQueueThread.hpp"

namespace al {
namespace {
class EffectCalcHandler {
public:
    virtual ~EffectCalcHandler();
    virtual void calcBegin();
    virtual void calc();
};

EffectCalcHandler* getEffectCalcHandler(const EffectSystem* pEffectSystem) {
    return *reinterpret_cast<EffectCalcHandler* const*>(
        reinterpret_cast<const u8*>(pEffectSystem) + 0x390);
}
}  // namespace

/**
 * Creates the actor group and the player holder of a scene.
 * @param maxActors The maximum number of actors.
 * @param maxPlayers The maximum number of players.
 * @param isUseMultiCore Whether the kit uses a multi core queue thread.
 * @param isUseAreaClipping Whether area clipping is used.
 */
LiveActorKit::LiveActorKit(s32 maxActors, s32 maxPlayers, bool isUseMultiCore,
                           bool isUseAreaClipping)
    : _0(maxActors), _4(maxPlayers), mAreaObjDirector(nullptr), mExecDirector(nullptr),
      mEffectSystem(nullptr), mGraphicsSystemInfo(nullptr), mCameraDirector(nullptr),
      mCameraDirectorRS(nullptr), mClippingDirector(nullptr), mCollisionDirector(nullptr),
      mItemDirector(nullptr), mPlayerHolder(nullptr), mSensorDirector(nullptr),
      mScreenPointDirector(nullptr), mShadowDirector(nullptr), mStageSwitchDirector(nullptr),
      mSwitchAreaDirector(nullptr), mActorGroup(nullptr), mDemoDirector(nullptr),
      mRumbleDirector(nullptr), mQueueThread(nullptr), _a0(isUseMultiCore),
      _a1(isUseAreaClipping) {
    mActorGroup = new LiveActorGroup("全てのアクター", maxActors);
    mPlayerHolder = new PlayerHolder(maxPlayers);
}

/**
 * Deletes the directors owned by the kit and detaches the effect system.
 */
LiveActorKit::~LiveActorKit() {
    delete mPlayerHolder;
    delete mActorGroup;
    delete mExecDirector;
    delete mSensorDirector;
    delete mSwitchAreaDirector;
    delete mClippingDirector;
    delete mQueueThread;
    if (mEffectSystem) {
        mEffectSystem->endScene();
        mEffectSystem->setCameraDirector(nullptr);
        mEffectSystem->setGraphicsSystemInfo(nullptr);
    }
}

/**
 * Creates the directors used by the actors of a scene.
 * @param unused Unused.
 * @param maxScreenPointTargets The number of screen point targets.
 * @param isUseMultiCoreExecute Whether the execute director uses multiple cores.
 */
void LiveActorKit::init(s32 unused, s32 maxScreenPointTargets, bool isUseMultiCoreExecute) {
    if (_a0) {
        mQueueThread = new MultiCoreQueueThread(5);
    }

    ExecuteSystemInitInfo initInfo;
    mExecDirector = new ExecuteDirector(_0, isUseMultiCoreExecute);
    mExecDirector->init(initInfo);
    mAreaObjDirector = new AreaObjDirector(_a0);
    s32 collisionThreadNum = _a0 ? -1 : 0;
    mCollisionDirector = new CollisionDirector(mExecDirector, collisionThreadNum);
    mCameraDirector = new CameraDirector(_4, mAreaObjDirector, mCollisionDirector);
    if (_a0) {
        mCameraDirectorRS = new CameraDirector_RS(2);
    }

    if (mEffectSystem) {
        mEffectSystem->setCameraDirector(mCameraDirector);
        mEffectSystem->initScene();
    }

    if (_a0) {
        mRumbleDirector = new PadRumbleDirector(mPlayerHolder, mCameraDirectorRS);
    } else {
        mRumbleDirector = new PadRumbleDirector(mPlayerHolder, mCameraDirector);
    }

    if (_a1) {
        ClippingAreaDirector* clippingDirector = new ClippingAreaDirector(
            mExecDirector, _0, mAreaObjDirector, mPlayerHolder,
            mCameraDirectorRS->getSceneCameraInfo(), mCameraDirectorRS, mQueueThread);
        mClippingDirector = clippingDirector;
        mCameraDirectorRS->setClippingDirector(clippingDirector);
    } else {
        mClippingDirector = new ClippingDirector(mExecDirector, _0, mAreaObjDirector, mPlayerHolder,
                                                 mCameraDirector->mSceneCameraInfo, nullptr);
    }

    mStageSwitchDirector = new StageSwitchDirector(mExecDirector, _a0);
    mStageSwitchDirector->mCameraDirector = mCameraDirectorRS;
    mScreenPointDirector = new ScreenPointDirector(maxScreenPointTargets);
}

/**
 * Creates the graphics system info.
 * @param rArg The graphics init arguments.
 * @param pStageName The stage name.
 */
void LiveActorKit::initGraphics(const GraphicsInitArg& rArg, const char* pStageName) {
    mGraphicsSystemInfo = new (0x10) GraphicsSystemInfo(pStageName);
    mGraphicsSystemInfo->init(rArg, this);
}

/**
 * Creates the hit sensor director.
 * @param scale The sensor scale.
 * @param unused Unused.
 */
void LiveActorKit::initHitSensorDirector(s32 scale, bool unused) {
    mSensorDirector = new HitSensorDirector(mExecDirector, scale, mQueueThread);
}

/**
 * Does nothing.
 */
void LiveActorKit::initShadowDirector() {}

/**
 * Creates the effect system.
 */
void LiveActorKit::initEffectSystem() {
    mEffectSystem = EffectSystem::initializeSystem(
        reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), nullptr,
        false);
}

/**
 * Finishes the initialization of the directors and the actors.
 * @param pChecker The scenario complete checker.
 */
void LiveActorKit::endInit(IScenarioCompleteChecker* pChecker) {
    mCollisionDirector->endInit();
    mClippingDirector->endInit();
    mAreaObjDirector->endInit();
    if (mEffectSystem) {
        mEffectSystem->startScene(mExecDirector);
    }
    if (mItemDirector) {
        mItemDirector->endInit();
    }
    if (mShadowDirector) {
        mShadowDirector->endInit();
    }
    if (mEffectSystem) {
        mEffectSystem->setGraphicsSystemInfo(mGraphicsSystemInfo);
    }
    mGraphicsSystemInfo->endInit();
    if (mEffectSystem) {
        mEffectSystem->endInit();
    }

    for (s32 i = 0; i < mActorGroup->mNumActors; i++) {
        LiveActor* actor = mActorGroup->mActors[i];
        actor->initAfterPlacement();
        if (actor->mShadowKeeper) {
            actor->mShadowKeeper->initAfterPlacement();
        }
    }

    mExecDirector->createExecutorListTable();
    mSwitchAreaDirector =
        SwitchAreaDirector::tryCreate(mAreaObjDirector, mPlayerHolder, mQueueThread);
    if (mSwitchAreaDirector) {
        mSwitchAreaDirector->endInit(pChecker);
    }
}

/**
 * Enables reduced buffer effect rendering depending on the rendering emitters.
 */
void LiveActorKit::updateReducedBufferEffect() {
    ViewRenderer* viewRenderer = mGraphicsSystemInfo->mViewRenderer;
    if (viewRenderer) {
        viewRenderer->setReducedEffectRender(mEffectSystem->isHasRenderingEmitter(0x100), false);
        viewRenderer->setReducedEffectRender(mEffectSystem->isHasRenderingEmitter(0x200), true);
    }
}

/**
 * Passes the area object director to the camera director.
 */
void LiveActorKit::setupCameraAreaObjDirector() {
    if (mCameraDirectorRS) {
        mCameraDirectorRS->setupCameraAreaObjDirector(mAreaObjDirector);
    }
}

/**
 * Updates the directors of the scene.
 */
void LiveActorKit::update() {
    if (mSwitchAreaDirector) {
        mSwitchAreaDirector->waitDone();
    }
    if (mGraphicsSystemInfo) {
        mGraphicsSystemInfo->clearGraphicsRequest();
    }
    if (mRumbleDirector) {
        mRumbleDirector->update();
    }
    if (mExecDirector) {
        mExecDirector->execute();
    }
    if (mGraphicsSystemInfo) {
        mGraphicsSystemInfo->updateGraphics(false);
    }
    if (mEffectSystem) {
        getEffectCalcHandler(mEffectSystem)->calc();
    }
    if (mAreaObjDirector) {
        mAreaObjDirector->update();
    }
    if (mSwitchAreaDirector) {
        mSwitchAreaDirector->update();
    }
}

/**
 * Clears the graphics requests.
 */
void LiveActorKit::clearGraphicsRequest() {
    if (mGraphicsSystemInfo) {
        mGraphicsSystemInfo->clearGraphicsRequest();
    }
}

/**
 * Updates the graphics system and the effects.
 * @param isPaused Whether the scene is paused.
 */
void LiveActorKit::updateGraphics(bool isPaused) {
    if (mGraphicsSystemInfo) {
        mGraphicsSystemInfo->updateGraphics(isPaused);
    }
    if (mEffectSystem) {
        getEffectCalcHandler(mEffectSystem)->calc();
    }
}

/**
 * Prepares the graphics system for drawing.
 * @return Whether drawing is possible.
 */
bool LiveActorKit::preDrawGraphics() {
    GameFrameworkNx* framework = GameFrameworkNx::sInstance;
    if (!framework->_27c && framework->_27b) {
        return false;
    }
    if (mGraphicsSystemInfo) {
        mGraphicsSystemInfo->preDrawGraphics(mCameraDirector->mSceneCameraInfo);
    }
    return true;
}

/**
 * Updates the pad rumble director.
 */
void LiveActorKit::updatePadRumble() {
    mRumbleDirector->update();
}
}  // namespace al
