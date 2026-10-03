#include "System/SaveDataAccessFunction.hpp"
#include "System/GameDataHolder.hpp"
#include "System/SaveDataAccessSequence.hpp"

namespace {
/**
 * @brief Advance an outstanding save operation until it finishes.
 * @param pSequence Non-null save sequence whose completion is required.
 */
inline void waitForSaveAccess(SaveDataAccessSequence* pSequence) {
    while (!pSequence->isDone()) {
        pSequence->updateNerve();
    }
}

} // namespace

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 */
void SaveDataAccessFunction::startSaveDataInit(GameDataHolder* pHolder) {
    pHolder->getSaveAccess()->startInit();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 */
void SaveDataAccessFunction::startSaveDataInitSync(GameDataHolder* pHolder) {
    pHolder->getSaveAccess()->startInitSync();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 */
void SaveDataAccessFunction::startSaveDataReadSync(GameDataHolder* pHolder) {
    pHolder->getSaveAccess()->startReadSync();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @return The current save-sequence condition.
 */
bool SaveDataAccessFunction::isWaitShowError(GameDataHolder* pHolder) {
    return pHolder->getSaveAccess()->isWaitShowError();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @return The current save-sequence condition.
 */
bool SaveDataAccessFunction::isEnableSave(const GameDataHolder* pHolder) {
    return pHolder->getSaveAccess()->isSaveEnabled();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @return The current save-sequence condition.
 */
bool SaveDataAccessFunction::isEnableHomeButtonMenu(GameDataHolder* pHolder) {
    return pHolder->getSaveAccess()->isEnableHomeButtonMenu();
}

/**
 * @brief Access the game's save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @return The current save-sequence condition.
 */
bool SaveDataAccessFunction::isWindowProcessingActive(GameDataHolder* pHolder) {
    return pHolder->getSaveAccess()->isWindowProcessingActive();
}

/**
 * @brief Update the save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param enabled True to allow future save operations; false to disable saving.
 */
void SaveDataAccessFunction::enableSave(GameDataHolder* pHolder, bool enabled) {
    pHolder->getSaveAccess()->enableSave(enabled);
}

/**
 * @brief Update the save sequence.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param option Current update option forwarded to the save-sequence state machine.
 * @return True when the sequence is idle after updating.
 */
bool SaveDataAccessFunction::updateSaveDataAccess(GameDataHolder* pHolder, bool option) {
    return pHolder->getSaveAccess()->update(option);
}

/**
 * @brief Start a save operation, optionally waiting for the previous one.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param wait True to complete any previous operation before starting.
 */
void SaveDataAccessFunction::startSaveDataRead(GameDataHolder* pHolder, bool wait) {
    if (wait) {
        waitForSaveAccess(pHolder->getSaveAccess());
    }
    pHolder->getSaveAccess()->startRead();
}

/**
 * @brief Start a save operation, optionally waiting for the previous one.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param wait True to complete any previous operation before starting.
 */
void SaveDataAccessFunction::startSaveDataWriteNoMessage(GameDataHolder* pHolder, bool wait) {
    if (wait) {
        waitForSaveAccess(pHolder->getSaveAccess());
    }
    pHolder->getSaveAccess()->startWriteNoMessage();
}

/**
 * @brief Start a save operation, optionally waiting for the previous one.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param option Write option passed to the save sequence.
 * @param fileId Save-file identifier for the write operation.
 * @param wait True to complete any previous operation before starting.
 */
void SaveDataAccessFunction::startSaveDataWrite(GameDataHolder* pHolder, bool option, int fileId, bool wait) {
    if (wait) {
        waitForSaveAccess(pHolder->getSaveAccess());
    }
    pHolder->getSaveAccess()->startWrite(option, fileId);
}

/**
 * @brief Start a save operation, optionally waiting for the previous one.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param option Select the alternate write operation.
 * @param wait True to complete any previous operation before starting.
 */
void SaveDataAccessFunction::startSaveDataWriteNoWindow(GameDataHolder* pHolder, bool option, bool wait) {
    if (wait) {
        waitForSaveAccess(pHolder->getSaveAccess());
    }
    pHolder->getSaveAccess()->startWriteNoWindow(option);
}

/**
 * @brief Write synchronously unless an idle-only request finds the sequence busy.
 * @param pHolder Non-null game-data holder with a created save sequence.
 * @param onlyWhenIdle True to skip the write when a previous operation is active.
 */
void SaveDataAccessFunction::startSaveDataWriteSync(GameDataHolder* pHolder, bool onlyWhenIdle) {
    if (!onlyWhenIdle || pHolder->getSaveAccess()->isDone()) {
        pHolder->getSaveAccess()->startWriteSync();
    }
}
