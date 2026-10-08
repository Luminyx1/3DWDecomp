#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Ids of the game's scene objects (al::getSceneObj / al::tryGetSceneObj).
 * @note Only ids whose object is known from reconstructed code are listed.
 */
enum SceneObjID : s32 {
    SceneObjID_CourseSelectDirector = 2,
    SceneObjID_DrcAssistDirectorList = 4,
    SceneObjID_GameDataHolder = 8,
    SceneObjID_GhostPlayerDirector = 9,
    SceneObjID_PlayerGroup = 16,
    SceneObjID_PlayerStocker = 18,
    SceneObjID_ScoreHolder = 20,
    SceneObjID_PlayerRetargettingSelector = 25,
    SceneObjID_FurEnv = 33,
    SceneObjID_ControllerEventWatcher = 34,
    SceneObjID_GoalItemHolder = 36,
    SceneObjID_CatGullHolder = 37,
    SceneObjID_OceanScenarioList = 42,
    SceneObjID_IslandKeeper = 44,
    SceneObjID_IslandMap = 45,
    SceneObjID_StampDirector = 48,
    SceneObjID_RaidonSurf = 51,
    SceneObjID_CloudBonusWatcher = 52,
    SceneObjID_SingleModeSceneLayout = 53,
    SceneObjID_PlayerKoopaJr = 56,
    SceneObjID_NekoParentHolder = 59,
};
