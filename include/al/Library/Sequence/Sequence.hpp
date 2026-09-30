#pragma once

#include <prim/seadSafeString.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Library/Scene/IUseSceneCreator.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace alSceneFunction {
class SceneFactory;
}

namespace al {
class AudioDirector;
class AudioKeeper;
class AudioSystemInfo;
struct DrawSystemInfo;
class ErrorViewer;
class GameDataHolderBase;
struct GameSystemInfo;
class Scene;
class ScreenCaptureExecutor;

struct SequenceInitInfo {
    SequenceInitInfo(const GameSystemInfo* pGameSystemInfo);

    const GameSystemInfo* mGameSystemInfo;
};

class Sequence : public NerveExecutor, public IUseAudioKeeper, public IUseSceneCreator {
public:
    Sequence(const char* pName);
    ~Sequence() override;

    virtual void init(const SequenceInitInfo& rInfo);
    virtual void update();
    virtual void kill();
    virtual void drawMain() const;
    virtual void drawSub() const;

    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }

    virtual bool isDisposable() const;
    virtual Scene* getCurrentScene() const;
    virtual ErrorViewer* getErrorViewer() const { return nullptr; }

    SceneCreator* getSceneCreator() const override { return mSceneCreator; }
    void setSceneCreator(SceneCreator* pSceneCreator) override { mSceneCreator = pSceneCreator; }

    void initAudio(const GameSystemInfo& rInfo, const char* pStageName, s32 seRequestNum,
                   s32 unused1, s32 unused2, const char* pKeeperName);
    void initAudioKeeper(const char* pName);
    void initDrawSystemInfo(const SequenceInitInfo& rInfo);
    AudioSystemInfo* getAudioSystemInfo() const;
    void doDrawScene(bool isForceDraw) const;

    const char* getName() const { return mName.cstr(); }
    bool isAlive() const { return mIsAlive; }
    Scene* getScene() const { return mCurrentScene; }
    Scene* getNextScene() const { return mNextScene; }
    AudioDirector* getAudioDirector() const { return mAudioDirector; }
    DrawSystemInfo* getDrawSystemInfo() const { return mDrawSystemInfo; }

    sead::FixedSafeString<0x40> mName;
    Scene* mCurrentScene = nullptr;
    Scene* mNextScene = nullptr;
    SceneCreator* mSceneCreator = nullptr;
    AudioDirector* mAudioDirector = nullptr;
    AudioKeeper* mAudioKeeper = nullptr;
    DrawSystemInfo* mDrawSystemInfo = nullptr;
    bool mIsAlive = true;
    bool _a9 = true;
    bool mIsChangeScene = true;
};

static_assert(sizeof(Sequence) == 0xb0);

void initSceneCreator(IUseSceneCreator* pUser, const SequenceInitInfo& rInfo,
                      GameDataHolderBase* pGameDataHolder, AudioDirector* pAudioDirector,
                      ScreenCaptureExecutor* pScreenCaptureExecutor,
                      alSceneFunction::SceneFactory* pSceneFactory);
Scene* createSceneAndInit(IUseSceneCreator* pUser, const char* pClassName,
                          const char* pStageName, s32 scenarioNo, const char* pSceneName);
Scene* createSceneAndUseInitThread(IUseSceneCreator* pUser, const char* pClassName,
                                   s32 priority, const char* pStageName, s32 scenarioNo,
                                   const char* pSceneName, const char* pHeapName);
void setSceneAndInit(IUseSceneCreator* pUser, Scene* pScene, const char* pStageName,
                     s32 scenarioNo, const char* pSceneName);
void setSceneAndUseInitThread(IUseSceneCreator* pUser, Scene* pScene, s32 priority,
                              const char* pStageName, s32 scenarioNo, const char* pSceneName,
                              sead::Heap* pHeap);
bool tryEndSceneInitThread(IUseSceneCreator* pUser);
bool isExistSceneInitThread(const IUseSceneCreator* pUser);
void setSequenceNameForActorPickTool(Sequence* pSequence, Scene* pScene);
}  // namespace al
