#pragma once

#include <prim/seadSafeString.h>

namespace al {
struct GameSystemInfo;
class AudioDirector;
class GameDataHolderBase;
class ScreenCaptureExecutor;

struct SceneInitInfo {
    SceneInitInfo(const GameSystemInfo* pGameSystemInfo, AudioDirector* pAudioDirector,
                  GameDataHolderBase* pGameDataHolder, ScreenCaptureExecutor* pScreenCaptureExecutor,
                  const char* pStageName, s32 scenarioNo, const char* pSceneName);

    const GameSystemInfo* mGameSystemInfo;
    AudioDirector* mAudioDirector;
    GameDataHolderBase* mGameDataHolder;
    ScreenCaptureExecutor* mScreenCaptureExecutor;
    const char* mStageName;
    s32 mScenarioNo;
    sead::FixedSafeString<512> mSceneName;
};

static_assert(sizeof(SceneInitInfo) == 0x248);
}  // namespace al
