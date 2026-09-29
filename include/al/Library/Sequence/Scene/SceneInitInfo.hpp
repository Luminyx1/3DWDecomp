#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.hpp>

namespace al {
struct GameSystemInfo;
class AudioDirector;
class GameDataHolderBase;
class ScreenCaptureExecutor;

struct SceneInitInfo {
    SceneInitInfo(const GameSystemInfo*, AudioDirector*, GameDataHolderBase*,
                  ScreenCaptureExecutor*, const char*, s32, const char*);

    const GameSystemInfo* mGameSystemInfo;          // _0
    AudioDirector* mAudioDirector;                  // _8
    GameDataHolderBase* mGameDataHolder;            // _10
    ScreenCaptureExecutor* mScreenCaptureExecutor;  // _18
    const char* mInitStageName;                     // _20
    s32 mScenarioNo;                                // _28
    sead::FixedSafeString<512> mSceneName;          // _30
};
}  // namespace al
