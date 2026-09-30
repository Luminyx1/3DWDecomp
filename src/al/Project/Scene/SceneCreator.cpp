#include "Project/Scene/SceneCreator.hpp"

#include <heap/seadHeapMgr.h>

#include "Library/Memory/HeapUtil.hpp"
#include "Library/Memory/SceneHeapSetter.hpp"
#include "Library/Scene/SceneFunction.hpp"
#include "Library/Scene/Scene.hpp"
#include "Library/Thread/AsyncFunctorThread.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Scene/SceneInitInfo.hpp"

namespace al {
void setCpuBoost(bool isBoost, bool isUnk);

using SceneInitFunctor =
    FunctorV1M<Scene*, void (Scene::*)(const SceneInitInfo&), const SceneInitInfo&>;

/**
 * Constructs a scene creator.
 * @param pGameSystemInfo game system info
 * @param pAudioDirector audio director
 * @param pGameDataHolder game data holder
 * @param pScreenCaptureExecutor screen capture executor
 * @param pSceneFactory scene factory
 */
SceneCreator::SceneCreator(const GameSystemInfo* pGameSystemInfo, AudioDirector* pAudioDirector,
                           GameDataHolderBase* pGameDataHolder,
                           ScreenCaptureExecutor* pScreenCaptureExecutor,
                           alSceneFunction::SceneFactory* pSceneFactory)
    : mGameSystemInfo(pGameSystemInfo), mGameDataHolder(pGameDataHolder),
      mAudioDirector(pAudioDirector), mScreenCaptureExecutor(pScreenCaptureExecutor),
      mSceneFactory(pSceneFactory) {}

/**
 * Creates a scene in the scene heap and initializes it, optionally on an initialize thread.
 * @param pClassName scene class name
 * @param pStageName stage name
 * @param scenarioNo scenario number
 * @param pSceneName scene name
 * @param isThreadInit whether the scene is initialized on an initialize thread
 * @param priority thread priority
 * @param pHeapName scene heap name, or null to use the stage name
 * @return created scene
 */
Scene* SceneCreator::createScene(const char* pClassName, const char* pStageName, s32 scenarioNo,
                                 const char* pSceneName, bool isThreadInit, s32 priority,
                                 const char* pHeapName) {
    const char* heapName = pHeapName ? pHeapName : pStageName;
    setCustomSceneHeapAlloc(mSceneFactory->tryGetCustomAlloc(pClassName));
    createSceneHeap(heapName);
    SceneHeapSetter setter;
    alSceneFunction::SceneFactory* factory = mSceneFactory;
    const char* name = factory->convertName(pClassName);
    const NameToCreator<alSceneFunction::SceneFunction>* entries = factory->mFuncs;
    s32 index = 0;

    while (!isEqualString(name, entries[index].name)) {
        index++;
    }

    Scene* scene = entries[index].func();
    SceneInitInfo* info = new SceneInitInfo(mGameSystemInfo, mAudioDirector, mGameDataHolder,
                                            mScreenCaptureExecutor, pStageName, scenarioNo,
                                            pSceneName);
    setCpuBoost(true, false);

    if (isThreadInit) {
        mInitThread = createAndStartInitializeThread(setter.mSceneHeap, priority,
                                                     SceneInitFunctor(scene, &Scene::init, *info));
    } else {
        scene->init(*info);
        setCpuBoost(false, false);
    }

    return scene;
}

/**
 * Creates the scene init info and initializes the scene on an initialize thread.
 * @param pScene scene
 * @param pStageName stage name
 * @param scenarioNo scenario number
 * @param pSceneName scene name
 * @param priority thread priority
 * @param pHeap heap to initialize the scene in, or null for the scene heap
 */
void SceneCreator::setSceneAndThreadInit(Scene* pScene, const char* pStageName, s32 scenarioNo,
                                         const char* pSceneName, s32 priority,
                                         sead::Heap* pHeap) {
    setCpuBoost(true, false);

    if (pHeap) {
        sead::ScopedCurrentHeapSetter setter(pHeap);
        SceneInitInfo* info =
            new SceneInitInfo(mGameSystemInfo, mAudioDirector, mGameDataHolder,
                              mScreenCaptureExecutor, pStageName, scenarioNo, pSceneName);
        mInitThread = createAndStartInitializeThread(
            pHeap, priority, SceneInitFunctor(pScene, &Scene::init, *info));
    } else {
        SceneHeapSetter setter;
        SceneInitInfo* info =
            new SceneInitInfo(mGameSystemInfo, mAudioDirector, mGameDataHolder,
                              mScreenCaptureExecutor, pStageName, scenarioNo, pSceneName);
        SceneInitFunctor functor(pScene, &Scene::init, *info);
        mInitThread = createAndStartInitializeThread(setter.mSceneHeap, priority, functor);
    }
}

/**
 * Creates the scene init info and initializes the scene.
 * @param pScene scene
 * @param pStageName stage name
 * @param scenarioNo scenario number
 * @param pSceneName scene name
 */
void SceneCreator::setSceneAndInit(Scene* pScene, const char* pStageName, s32 scenarioNo,
                                   const char* pSceneName) {
    SceneHeapSetter setter;
    setCpuBoost(true, false);
    SceneInitInfo* info = new SceneInitInfo(mGameSystemInfo, mAudioDirector, mGameDataHolder,
                                            mScreenCaptureExecutor, pStageName, scenarioNo,
                                            pSceneName);
    pScene->init(*info);
    setCpuBoost(false, false);
}

/**
 * Waits for the initialize thread and destroys it once it is done.
 * @return whether no initialize thread is running
 */
bool SceneCreator::tryEndInitThread() {
    if (mInitThread) {
        if (!tryWaitDoneAndDestroyInitializeThread(mInitThread)) {
            return false;
        }

        setCpuBoost(false, false);
        mInitThread = nullptr;
    }

    return true;
}

/**
 * Checks if an initialize thread exists.
 * @return whether an initialize thread exists
 */
bool SceneCreator::isExistInitThread() const {
    return mInitThread != nullptr;
}
}  // namespace al
