#include "Library/Sequence/Scene/SceneInitInfo.hpp"

namespace al {
/**
 * @brief Constructs the information handed to a scene when it is initialized.
 * @param pGameSystemInfo The game's system info.
 * @param pAudioDirector The audio director shared with the scene.
 * @param pGameDataHolder The game data holder.
 * @param pScreenCaptureExecutor The executor used for screen captures.
 * @param pInitStageName The name of the stage the scene starts in.
 * @param scenarioNo The scenario number of the starting stage.
 * @param pInitSceneName The name of the scene.
 */
SceneInitInfo::SceneInitInfo(const GameSystemInfo* pGameSystemInfo,
                             AudioDirector* pAudioDirector, GameDataHolderBase* pGameDataHolder,
                             ScreenCaptureExecutor* pScreenCaptureExecutor,
                             const char* pInitStageName, s32 scenarioNo,
                             const char* pInitSceneName)
    : mGameSystemInfo(pGameSystemInfo), mAudioDirector(pAudioDirector),
      mGameDataHolder(pGameDataHolder), mScreenCaptureExecutor(pScreenCaptureExecutor),
      mInitStageName(pInitStageName), mScenarioNo(scenarioNo) {
    mSceneName = pInitSceneName;
}
}  // namespace al
