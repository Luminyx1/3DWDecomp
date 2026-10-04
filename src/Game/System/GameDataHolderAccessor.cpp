#include "System/GameDataHolderAccessor.hpp"
#include "Library/Scene/SceneObjHolder.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Scene/SceneObjID.hpp"

/**
 * @brief Retrieves the game-data holder from an object's scene registry.
 * @param pUser Non-null scene user whose registry contains the game-data holder.
 */
GameDataHolderAccessor::GameDataHolderAccessor(const al::IUseSceneObjHolder* pUser)
    : mSceneObj(al::getSceneObj(pUser, SceneObjID_GameDataHolder)) {}

/**
 * @brief Retrieves the game-data holder directly from a scene registry.
 * @param pHolder Non-null scene registry containing the game-data holder.
 */
GameDataHolderAccessor::GameDataHolderAccessor(const al::SceneObjHolder* pHolder)
    : mSceneObj(pHolder->getObj(SceneObjID_GameDataHolder)) {}
