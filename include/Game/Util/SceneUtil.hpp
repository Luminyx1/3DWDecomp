#pragma once

namespace al {
class Scene;
}  // namespace al

namespace rc {
bool isActiveDemoCamera(const al::Scene* pScene);
bool isActiveDemoMovingCamera(const al::Scene* pScene);
bool isActiveDemoIntro(const al::Scene* pScene);
bool isActiveDemoPlayer(const al::Scene* pScene);
bool isActiveDemoBinding(const al::Scene* pScene);
bool isActiveDemoCutscene(const al::Scene* pScene);
bool isActiveDemoInGameCutscene(const al::Scene* pScene);
bool isActiveDemoPlayerCutscene(const al::Scene* pScene);
bool isSingleModeScene(const al::Scene* pScene);
bool isSingleModeScene(const char* pSceneName);
bool isSingleModeBossScene(const al::Scene* pScene);
bool isSingleModeBossScene(const char* pSceneName);
bool isSingleModeBossBattleScene(const al::Scene* pScene);
}  // namespace rc
