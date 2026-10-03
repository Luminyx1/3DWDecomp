#pragma once

#include "System/GameDataHolder.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"

class GameDataHolderAccessor {
  public:
    GameDataHolderAccessor(const al::IUseSceneObjHolder* pUser);
    GameDataHolderAccessor(const al::SceneObjHolder* pHolder);

    /**
     * @brief Wrap an existing game-data holder.
     * @param pHolder Game-data holder to access; must remain alive while used.
     */
    explicit GameDataHolderAccessor(GameDataHolder* pHolder) : mSceneObj(pHolder) {}

    /**
     * @brief Access the typed game-data holder.
     * @return The wrapped game-data holder.
     */
    GameDataHolder* getHolder() const { return static_cast<GameDataHolder*>(mSceneObj); }

    al::ISceneObj* mSceneObj; // 0x00
};
