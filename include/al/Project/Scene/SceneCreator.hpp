#pragma once

#include <basis/seadTypes.h>

namespace sead {
class Heap;
}

namespace alSceneFunction {
class SceneFactory;
}

namespace al {
struct GameSystemInfo;
class AudioDirector;
class GameDataHolderBase;
class InitializeThread;
class Scene;
class ScreenCaptureExecutor;

class SceneCreator {
public:
    SceneCreator(const GameSystemInfo* pGameSystemInfo, AudioDirector* pAudioDirector,
                 GameDataHolderBase* pGameDataHolder, ScreenCaptureExecutor* pScreenCaptureExecutor,
                 alSceneFunction::SceneFactory* pSceneFactory);

    Scene* createScene(const char* pClassName, const char* pStageName, s32 scenarioNo,
                       const char* pSceneName, bool isThreadInit, s32 priority,
                       const char* pHeapName);
    void setSceneAndThreadInit(Scene* pScene, const char* pStageName, s32 scenarioNo,
                               const char* pSceneName, s32 priority, sead::Heap* pHeap);
    void setSceneAndInit(Scene* pScene, const char* pStageName, s32 scenarioNo,
                         const char* pSceneName);
    bool tryEndInitThread();
    bool isExistInitThread() const;

    const GameSystemInfo* mGameSystemInfo;
    GameDataHolderBase* mGameDataHolder;
    AudioDirector* mAudioDirector;
    ScreenCaptureExecutor* mScreenCaptureExecutor;
    alSceneFunction::SceneFactory* mSceneFactory;
    InitializeThread* mInitThread = nullptr;
};
}  // namespace al
