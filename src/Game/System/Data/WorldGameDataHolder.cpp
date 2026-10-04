#include "System/Data/WorldGameDataHolder.hpp"

/**
 * @brief Creates a world record with every item and demo flag clear.
 */
WorldGameData::WorldGameData() = default;

/**
 * @brief Clears every item and demo flag.
 */
void WorldGameData::initialize() { mFlags.makeAllZero(); }

/**
 * @brief Checks a world item flag.
 * @param itemIndex Valid bit index from 0 to 63; no runtime bounds check is performed.
 * @return True when the selected bit is set.
 */
bool WorldGameData::isOnItemFlag(s32 itemIndex) const { return mFlags.isOnBit(itemIndex); }

/**
 * @brief Sets a world item flag when its index is valid.
 * @param itemIndex Bit index from 0 to 63; out-of-range indices are ignored.
 */
void WorldGameData::setItemFlag(s32 itemIndex) {
    if (static_cast<u32>(itemIndex) < 64) {
        mFlags.setBit(itemIndex);
    }
}

/**
 * @brief Clears item flags 0 through 49, preserving the remaining world flags.
 */
void WorldGameData::resetItemFlag() { clearItemFlags(50); }

/**
 * @brief Clears item flags 0 through 50, preserving the remaining world flags.
 */
void WorldGameData::resetAllItemFlag() { clearItemFlags(51); }

/**
 * @brief Checks whether the world's first demo has been shown.
 * @return True when demo flag 61 is set.
 */
bool WorldGameData::isShowFirstDemo() const { return mFlags.isOnBit(61); }

/**
 * @brief Records that the world's first demo has been shown.
 */
void WorldGameData::setShowFirstDemoFlag() { mFlags.setBit(61); }

/**
 * @brief Allocate progress for the twelve worlds.
 */
WorldGameDataHolder::WorldGameDataHolder() { mpWorlds = new WorldGameData[cWorldNum]; }

/**
 * @brief Reset all world progress.
 */
void WorldGameDataHolder::initialize() {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].initialize();
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
        mpWorlds[i].resetItemFlag();
    }
}

/**
 * @brief Clear item flags 0 through 50 in every world.
 */
void WorldGameDataHolder::resetAllItemFlag() {
    for (s32 i = 0; i < cWorldNum; ++i) {
        mpWorlds[i].resetAllItemFlag();
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
