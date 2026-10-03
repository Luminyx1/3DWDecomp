#include "System/GameDataFileBase.hpp"
#include "System/ControlUserDataHolder.hpp"

/**
 * @brief Update whether this file is playing a stage.
 */
void GameDataFileBase::onStageStart() { mStageStarted = true; }

/**
 * @brief Update whether this file is playing a stage.
 */
void GameDataFileBase::onStageEnd() { mStageStarted = false; }

/**
 * @brief Access a player's persistent control-user record.
 * @param userId Control-user index from 0 through 3.
 * @return The selected control-user record.
 */
const ControlUserData* GameDataFileBase::getControlUserData(int userId) const {
    return mpUsers->getControlUserData(userId);
}

/**
 * @brief Enter a player with the requested character.
 * @param userId Control-user index from 0 through 3.
 * @param characterType Requested character; -1 preserves the current selection.
 * @return True when the character can be assigned.
 */
bool GameDataFileBase::entryPlayer(int userId, int characterType) {
    return mpUsers->entryPlayer(userId, characterType);
}

/**
 * @brief Read the main player's character selection.
 * @return The main player's character identifier.
 */
int GameDataFileBase::getMainPlayerCharacterType() const {
    return mpUsers->getControlUserData(mMainUserId)->mCharacterType;
}

/**
 * @brief Read the main player's character selection.
 * @return The main player's character identifier.
 */
int GameDataFileBase::getMainPlayerCharacterTypeSaved() const {
    return mpUsers->getControlUserData(mMainUserId)->mSavedCharacterType;
}

/**
 * @brief Add coins and consume one hundred when the total crosses the threshold.
 * @param count Signed coins to add; this operation awards at most one life.
 * @return True when one hundred coins were consumed.
 */
bool GameDataFileBase::addCoin(int count) {
    mCoinCount += count;
    if (mCoinCount >= 100) {
        mCoinCount -= 100;
        return true;
    }
    return false;
}

/**
 * @brief Handle life initialization for a mode without a life counter.
 * @param life Requested life count; unused in the base implementation.
 */
void GameDataFileBase::initPlayerLife(int life) {}

/**
 * @brief Read the base mode's unused life counter.
 * @return Zero; derived game modes may override this.
 */
int GameDataFileBase::getPlayerLife() const { return 0; }

/**
 * @brief Handle a life award for a mode without a life counter.
 * @param life Requested life increment; unused in the base implementation.
 * @return False because the base mode has no lives to update.
 */
bool GameDataFileBase::addPlayerLife(int life) { return false; }
