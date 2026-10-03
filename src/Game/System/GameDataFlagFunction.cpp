#include "System/GameDataFlagFunction.hpp"
#include "System/GameDataFile.hpp"

/**
 * @brief Check whether Rosetta has been unlocked in 3D World.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the active 3D World file has unlocked Rosetta.
 */
bool GameDataFlagFunction::isAlreadyOpenRosetta(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }
    return (accessor.getHolder()->getPlayingFile()->getGameFlag() & (1ull << 5)) != 0;
}

/**
 * @brief Check a character's all-clear flag in the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 * @return True when the character's all-clear flag is set.
 */
bool GameDataFlagFunction::isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor,
                                                          int characterType) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }
    return (static_cast<u32>(accessor.getHolder()->getPlayingFile()->getGameFlag()) &
            (1u << characterType)) != 0;
}
