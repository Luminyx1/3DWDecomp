#include "Project/Scene/SceneInitInfo.hpp"

namespace al {
/**
 * Constructs the scene init info.
 * @param pGameSystemInfo game system info
 * @param pAudioDirector audio director
 * @param pGameDataHolder game data holder
 * @param pScreenCaptureExecutor screen capture executor
 * @param pStageName stage name
 * @param scenarioNo scenario number
 * @param pSceneName scene name
 */
SceneInitInfo::SceneInitInfo(const GameSystemInfo* pGameSystemInfo, AudioDirector* pAudioDirector,
                             GameDataHolderBase* pGameDataHolder,
                             ScreenCaptureExecutor* pScreenCaptureExecutor, const char* pStageName,
                             s32 scenarioNo, const char* pSceneName)
    : mGameSystemInfo(pGameSystemInfo), mAudioDirector(pAudioDirector),
      mGameDataHolder(pGameDataHolder), mScreenCaptureExecutor(pScreenCaptureExecutor),
      mStageName(pStageName), mScenarioNo(scenarioNo) {
    mSceneName = pSceneName;
}
}  // namespace al
