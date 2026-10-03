#include "System/SelectedGameDataFlagFunction.hpp"
#include "System/GameDataFile.hpp"

/**
 * @brief Check a progression flag in the selected save file.
 * @param pFile Non-null save file to inspect.
 * @return True when the progression flag is set.
 */
bool SelectedGameDataFlagFunction::isAlreadyOpenRosetta(const GameDataFile* pFile) {
    return (pFile->getGameFlag() & (1ull << 5)) != 0;
}

/**
 * @brief Check a progression flag in the selected save file.
 * @param pFile Non-null save file to inspect.
 * @return True when the progression flag is set.
 */
bool SelectedGameDataFlagFunction::isAlreadyShowSpecialWorldOpenDemo(const GameDataFile* pFile) {
    return (pFile->getGameFlag() & (1ull << 13)) != 0;
}

/**
 * @brief Check the all-clear flag for a character.
 * @param pFile Non-null save file to inspect.
 * @param characterType Playable character identifier from 0 through 4.
 * @return True when the character's all-clear flag is set.
 */
bool SelectedGameDataFlagFunction::isAlreadyOpenAllClearCharacter(const GameDataFile* pFile,
                                                                  int characterType) {
    return (static_cast<u32>(pFile->getGameFlag()) & (1u << characterType)) != 0;
}
