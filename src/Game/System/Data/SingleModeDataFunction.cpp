#include "System/Data/SingleModeDataFunction.hpp"
#include "System/Data/SingleModeData.hpp"

/**
 * @brief Read progression from the active single-mode file.
 * @param accessor Accessor to an initialized single-mode game-data holder.
 * @return The requested single-mode progression value.
 */
int SingleModeDataFunction::getUnlockedPhase(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSingleFile()->getUnlockedPhase();
}

/**
 * @brief Read progression from the active single-mode file.
 * @param accessor Accessor to an initialized single-mode game-data holder.
 * @return The requested single-mode progression value.
 */
int SingleModeDataFunction::getUnlockedIslandNum(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSingleFile()->getUnlockedIslandNum();
}

/**
 * @brief Read progression from the active single-mode file.
 * @param accessor Accessor to an initialized single-mode game-data holder.
 * @return The requested single-mode progression value.
 */
int SingleModeDataFunction::getLastValidIslandVisited(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSingleFile()->getLastValidIslandVisited();
}

/**
 * @brief Read progression from the active single-mode file.
 * @param accessor Accessor to an initialized single-mode game-data holder.
 * @return The requested single-mode progression value.
 */
int SingleModeDataFunction::getCurValidIslandVisited(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSingleFile()->getCurValidIslandVisited();
}
