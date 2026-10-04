#include "Util/SceneUtil.hpp"
#include "Demo/ProjectDemoDirector.hpp"
#include "Library/Scene/Scene.hpp"
#include "Library/Scene/SceneUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Scene/SingleModeScene.hpp"

namespace rc {

/**
 * @brief Checks whether the scene's active demo is a camera demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the camera demo name.
 */
bool isActiveDemoCamera(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameCamera(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a moving-camera demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the moving-camera demo name.
 */
bool isActiveDemoMovingCamera(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameMovingCamera(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a intro demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the intro demo name.
 */
bool isActiveDemoIntro(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameIntro(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a player demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the player demo name.
 */
bool isActiveDemoPlayer(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNamePlayer(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a binding demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the binding demo name.
 */
bool isActiveDemoBinding(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameBinding(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a cutscene demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the cutscene demo name.
 */
bool isActiveDemoCutscene(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameCutscene(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a in-game cutscene demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the in-game cutscene demo name.
 */
bool isActiveDemoInGameCutscene(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNameInGameCutscene(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether the scene's active demo is a player cutscene demo.
 * @param pScene Scene to check.
 * @return true if a demo is active and its name is the player cutscene demo name.
 */
bool isActiveDemoPlayerCutscene(const al::Scene* pScene) {
    if (!al::isActiveDemo(pScene)) {
        return false;
    }

    return al::isEqualString(ProjectDemoDirector::getDemoNamePlayerCutscene(), al::getActiveDemoName(pScene));
}

/**
 * @brief Checks whether a scene is one of Bowser's Fury's single-mode scenes.
 * @param pScene Scene to check.
 * @return true if the scene's name is a single-mode scene name.
 */
bool isSingleModeScene(const al::Scene* pScene) {
    return isSingleModeScene(pScene->getName());
}

/**
 * @brief Checks whether a scene name is one of Bowser's Fury's single-mode scenes.
 * @param pSceneName Scene name to check.
 * @return true if the name is a single-mode scene name.
 */
bool isSingleModeScene(const char* pSceneName) {
    return al::isEqualString(pSceneName, "SingleModeScene") ||
           al::isEqualString(pSceneName, "IslandScene") ||
           al::isEqualString(pSceneName, "PhaseScene") ||
           al::isEqualString(pSceneName, "Phase0Scene") ||
           al::isEqualString(pSceneName, "Phase1Scene") ||
           al::isEqualString(pSceneName, "Phase2Scene") ||
           al::isEqualString(pSceneName, "Phase3Scene") ||
           al::isEqualString(pSceneName, "Phase4V2Scene") ||
           al::isEqualString(pSceneName, "PrOceanScene") ||
           isSingleModeBossScene(pSceneName);
}

/**
 * @brief Checks whether a scene is one of Bowser's Fury's boss scenes.
 * @param pScene Scene to check.
 * @return true if the scene's name is a single-mode boss scene name.
 */
bool isSingleModeBossScene(const al::Scene* pScene) {
    return isSingleModeBossScene(pScene->getName());
}

/**
 * @brief Checks whether a scene name is one of Bowser's Fury's boss scenes.
 * @param pSceneName Scene name to check.
 * @return true if the name is a boss or Plessie-chase phase scene name.
 */
bool isSingleModeBossScene(const char* pSceneName) {
    return al::isEqualString(pSceneName, "PhaseBossScene") ||
           al::isEqualString(pSceneName, "PhaseBossV2Scene") ||
           al::isEqualString(pSceneName, "PhasePlessieChaseScene") ||
           al::isEqualString(pSceneName, "PhasePlessieChaseV2Scene");
}

/**
 * @brief Checks whether a scene is a single-mode scene currently in a boss battle.
 * @param pScene Scene to check.
 * @return true if the scene is a single-mode scene and reports a boss battle.
 */
bool isSingleModeBossBattleScene(const al::Scene* pScene) {
    if (!isSingleModeScene(pScene)) {
        return false;
    }

    return static_cast<const SingleModeScene*>(pScene)->isBossScene();
}

}  // namespace rc
