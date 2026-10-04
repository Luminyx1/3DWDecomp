#include "System/Data/WorldGameDataHolder.hpp"

/**
 * @brief Allocate progress for the twelve worlds.
 */
WorldGameDataHolder::WorldGameDataHolder() { mpWorlds = new WorldGameData[cWorldNum]; }

/**
 * @brief Reset all world progress.
 */
void WorldGameDataHolder::initialize() {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].clearAllFlags();
    }
}

/**
 * @brief Copy progress for every world.
 * @param pOther Non-null source holder containing twelve world records.
 */
void WorldGameDataHolder::copy(const WorldGameDataHolder* pOther) {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].copyFlags(pOther->mpWorlds[i]);
    }
}

/**
 * @brief Access progress for a world.
 * @param worldId One-based world identifier from 1 through 12.
 * @return The selected world record.
 */
WorldGameData* WorldGameDataHolder::getWorldGameData(int worldId) { return &mpWorlds[worldId - 1]; }

/**
 * @brief Clear item flags 0 through 49 in every world.
 */
void WorldGameDataHolder::resetItemFlag() {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].clearItemFlags(50);
    }
}

/**
 * @brief Clear item flags 0 through 50 in every world.
 */
void WorldGameDataHolder::resetAllItemFlag() {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].clearItemFlags(51);
    }
}

/**
 * @brief Load every world's flags from a save stream.
 * @param pStream Non-null stream positioned at the world records.
 * @return Always true.
 */
bool WorldGameDataHolder::readFromStream(sead::ReadStream* pStream) {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].readFlags(pStream);
    }

    return true;
}

/**
 * @brief Store every world's flags to a save stream, or skip over them.
 * @param pStream Non-null stream positioned at the world records.
 * @param isSkip True to advance past the records without writing them.
 */
void WorldGameDataHolder::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].writeFlags(pStream, isSkip);
    }
}

/**
 * @brief Identify the one-up item flag in a mushroom house.
 * @return The one-up item flag index.
 */
int WorldGameDataFunction::getKinokoOneUpItemId() { return 50; }
