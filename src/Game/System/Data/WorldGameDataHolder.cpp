#include "System/Data/WorldGameDataHolder.hpp"

/**
 * @brief Allocate progress for the twelve worlds.
 */
WorldGameDataHolder::WorldGameDataHolder() { mpWorlds = new WorldGameData[12]; }

/**
 * @brief Reset all world progress.
 */
void WorldGameDataHolder::initialize() {
    for (int i = 0; i < 12; ++i) {
        mpWorlds[i].initialize();
    }
}

/**
 * @brief Copy progress for every world.
 * @param pOther Non-null source holder containing twelve world records.
 */
void WorldGameDataHolder::copy(const WorldGameDataHolder* pOther) {
    for (int i = 0; i < 12; ++i) {
        mpWorlds[i] = pOther->mpWorlds[i];
    }
}

/**
 * @brief Reset the corresponding collectible flags in every world.
 */
void WorldGameDataHolder::resetItemFlag() {
    for (int i = 0; i < 12; ++i) {
        mpWorlds[i].resetItemFlag();
    }
}

/**
 * @brief Reset the corresponding collectible flags in every world.
 */
void WorldGameDataHolder::resetAllItemFlag() {
    for (int i = 0; i < 12; ++i) {
        mpWorlds[i].resetAllItemFlag();
    }
}

/**
 * @brief Access progress for a world.
 * @param worldId One-based world identifier from 1 through 12.
 * @return The selected world record.
 */
WorldGameData* WorldGameDataHolder::getWorldGameData(int worldId) { return &mpWorlds[worldId - 1]; }

/**
 * @brief Identify the one-up item flag in a mushroom house.
 * @return The one-up item flag index.
 */
int WorldGameDataFunction::getKinokoOneUpItemId() { return 50; }
