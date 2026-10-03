#include "System/IslandSaveDataHolder.hpp"

/**
 * @brief Allocate island progress and mark the initial island as visited.
 * @param count Positive number of island records to allocate.
 */
IslandSaveDataHolder::IslandSaveDataHolder(int count) : mCount(count) {
    mpIslands = new IslandSaveData[count];
    initialize();
}

/**
 * @brief Clear all island progress and mark island zero as visited.
 */
void IslandSaveDataHolder::initialize() {
    for (int i = 0; i < mCount; ++i) {
        mpIslands[i].initialize();
    }
    mpIslands[0].mFlags |= 1;
}

/**
 * @brief Copy saved progress for all islands.
 * @param rOther Source holder with at least as many records as this holder.
 */
void IslandSaveDataHolder::copy(const IslandSaveDataHolder& rOther) {
    for (int i = 0; i < mCount; ++i) {
        mpIslands[i].copy(rOther.mpIslands[i]);
    }
}

/**
 * @brief Check island progression state.
 * @param islandId Island index within the allocated record count.
 * @return True when the requested progression condition holds.
 */
bool IslandSaveDataHolder::isIslandFirstVisit(int islandId) const {
    return (mpIslands[islandId].mFlags & 1) == 0;
}

/**
 * @brief Check island progression state.
 * @param islandId Island index within the allocated record count.
 * @return True when the requested progression condition holds.
 */
bool IslandSaveDataHolder::isIslandUnlocked(int islandId) const {
    return (mpIslands[islandId].mFlags & 2) != 0;
}

/**
 * @brief Record island progression.
 * @param islandId Island index within the allocated record count.
 */
void IslandSaveDataHolder::setIslandVisited(int islandId) { mpIslands[islandId].mFlags |= 1; }

/**
 * @brief Record island progression.
 * @param islandId Island index within the allocated record count.
 */
void IslandSaveDataHolder::setIslandUnlocked(int islandId) { mpIslands[islandId].mFlags |= 2; }

/**
 * @brief Query whether vandalism has been recorded for an island phase.
 * @param islandId Island index within the allocated record count.
 * @param phase Phase bit from 0 through 7; other values return false.
 * @param pActive Non-null output written only when vandalism has been recorded.
 * @return True when a vandalism record exists for the phase.
 */
bool IslandSaveDataHolder::isIslandVandalized(int islandId, int phase, bool* pActive) const {
    if (static_cast<unsigned int>(phase) >= 8) {
        return false;
    }
    const auto& rIsland = mpIslands[islandId];
    const u32 mask = 1u << phase;
    if ((rIsland.mStateBytes[1] & mask) != 0) {
        *pActive = (rIsland.mStateBytes[2] & mask) != 0;
        return true;
    }
    return false;
}

/**
 * @brief Update both recorded and active vandalism flags.
 * @param islandId Island index within the allocated record count.
 * @param phase Phase bit from 0 through 7.
 */
void IslandSaveDataHolder::setIslandVandalized(int islandId, int phase) {
    const u32 mask = 1u << phase;
    mpIslands[islandId].mStateBytes[1] |= mask;
    mpIslands[islandId].mStateBytes[2] |= mask;
}

/**
 * @brief Update both recorded and active vandalism flags.
 * @param islandId Island index within the allocated record count.
 * @param phase Phase bit from 0 through 7.
 */
void IslandSaveDataHolder::clearIslandVandalized(int islandId, int phase) {
    const u32 mask = 1u << phase;
    mpIslands[islandId].mStateBytes[1] &= ~mask;
    mpIslands[islandId].mStateBytes[2] &= ~mask;
}

/**
 * @brief Access an island progress record.
 * @param islandId Island index within the allocated record count.
 * @return The selected island record.
 */
const IslandSaveData* IslandSaveDataHolder::getIslandSaveData(int islandId) const {
    return &mpIslands[islandId];
}

/**
 * @brief Access an island progress record.
 * @param islandId Island index within the allocated record count.
 * @return The selected island record.
 */
IslandSaveData* IslandSaveDataHolder::getIslandSaveDataPtr(int islandId) { return &mpIslands[islandId]; }
