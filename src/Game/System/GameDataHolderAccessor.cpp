#include "System/GameDataHolderAccessor.hpp"
#include "Library/Scene/SceneObjHolder.hpp"
#include "Library/Scene/SceneObjUtil.hpp"

namespace {
constexpr int cGameDataHolderSceneObjId = 8;
}

/**
 * @brief Retrieves the game-data holder from an object's scene registry.
 * @param pUser Non-null scene user whose registry contains scene object 8.
 */
GameDataHolderAccessor::GameDataHolderAccessor(const al::IUseSceneObjHolder* pUser)
    : mSceneObj(al::getSceneObj(pUser, cGameDataHolderSceneObjId)) {}

/**
 * @brief Retrieves the game-data holder directly from a scene registry.
 * @param pHolder Non-null scene registry containing scene object 8.
 */
GameDataHolderAccessor::GameDataHolderAccessor(const al::SceneObjHolder* pHolder)
    : mSceneObj(pHolder->getObj(cGameDataHolderSceneObjId)) {}
