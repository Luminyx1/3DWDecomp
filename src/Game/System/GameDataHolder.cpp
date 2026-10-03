#include "System/GameDataHolder.hpp"
#include "System/GameDataCommon.hpp"

/**
 * @brief Identify the game-data scene object.
 * @return The scene-object name.
 */
const char* GameDataHolder::getSceneObjName() const {
    return "\u30b2\u30fc\u30e0\u30c7\u30fc\u30bf\u4fdd\u6301";
}

/**
 * @brief Bind the scene-object holder.
 * @param pHolder Scene-object holder used by subsequent game-data operations.
 */
void GameDataHolder::setSceneObjHolder(al::SceneObjHolder* pHolder) { mpSceneObjHolder = pHolder; }

/**
 * @brief Access a save-file slot.
 * @param fileId Save-file slot from 0 through 3.
 * @return The save data belonging to the requested slot.
 */
GameDataFile* GameDataHolder::getGameDataFile(int fileId) const { return mppFiles[fileId]; }

/**
 * @brief Access a save-file slot.
 * @param fileId Save-file slot from 0 through 3.
 * @return The save data belonging to the requested slot.
 */
SingleModeData* GameDataHolder::getSingleModeDataFile(int fileId) const { return mppSingleFiles[fileId]; }

/**
 * @brief Remember the last game mode.
 * @param mode Game mode to persist in common save data.
 */
void GameDataHolder::setLastPlayedMode(GameMode mode) { mpCommon->mValues[0] = static_cast<u32>(mode); }

/**
 * @brief Read the last game mode.
 * @return The saved game-mode identifier.
 */
GameMode GameDataHolder::getLastPlayedMode() const { return static_cast<GameMode>(mpCommon->mValues[0]); }

/**
 * @brief Read the last selected save-file slot.
 * @return The saved slot index.
 */
int GameDataHolder::getLastPlayingFileId() const { return mpCommon->mValues[1]; }

/**
 * @brief Read the last selected save-file slot.
 * @return The saved slot index.
 */
int GameDataHolder::getLastSingleModePlayingFileID() const { return mpCommon->mValues[2]; }

/**
 * @brief Check whether Luigi Bros. has been unlocked.
 * @return True when the common unlock flag is set.
 */
bool GameDataHolder::isUnlockLuigiBros() const { return mpCommon->mState; }

/**
 * @brief Unlock Luigi Bros. in common save data.
 */
void GameDataHolder::unlockLuigiBros() { mpCommon->mState = true; }

/**
 * @brief Select the game data used by the title demonstration.
 * @param pFile Game-data file used while the title demonstration runs.
 */
void GameDataHolder::setGameFileForTitleDemo(GameDataFile* pFile) { mpPlayingFile = pFile; }
