#include "System/Data/WorldGameData.hpp"

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
