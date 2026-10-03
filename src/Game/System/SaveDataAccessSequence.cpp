#include "System/SaveDataAccessSequence.hpp"

/**
 * @brief Enable or disable future save requests.
 * @param enabled True to allow saving; false to reject future write requests.
 */
void SaveDataAccessSequence::enableSave(bool enabled) { mSaveDisabled = !enabled; }

/**
 * @brief Report the ghost save-data size, which is unused in this build.
 * @param worldId World identifier; unused because ghost saves are disabled.
 * @return Zero bytes.
 */
int SaveDataAccessSequence::calcGhostSaveDataSize(int worldId) const { return 0; }

/**
 * @brief Handle an operation that requires no work in this build.
 */
void SaveDataAccessSequence::exeIdle() {}

/**
 * @brief Handle an operation that requires no work in this build.
 */
void SaveDataAccessSequence::makeSaveGhostWorldList() {}
