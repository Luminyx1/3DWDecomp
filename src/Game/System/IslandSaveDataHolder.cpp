#include "System/IslandSaveDataHolder.hpp"
#include <attributes.h>
#include <stream/seadStream.h>

namespace {

/**
 * @brief Serialized form of one island record as stored in the save file.
 */
struct IslandSaveRecord {
    u16 mFlags;
    u64 mCompletedScenarios;
    u8 mStateBytes[3];
    u8 mProgressBytes[8];
    s32 mCurActiveScenario;
    u8 mReserved[8];
};

static_assert(sizeof(IslandSaveRecord) == 0x28);

/**
 * @brief Clear one island record (same as IslandSaveData::initialize).
 * @param rIsland Record to clear.
 */
ALWAYS_INLINE void clearIsland(IslandSaveData& rIsland) {
    rIsland.mFlags = 0;
    rIsland.mCompletedScenarios = 0;
    for (auto& rByte : rIsland.mStateBytes) {
        rByte = 0;
    }

    for (auto& rByte : rIsland.mProgressBytes) {
        rByte = 0;
    }

    rIsland.mCurActiveScenario = 0;
}

/**
 * @brief Copy persistent island progress (same as IslandSaveData::copy).
 * @param rDst Destination record.
 * @param rSrc Source record.
 */
ALWAYS_INLINE void copyIsland(IslandSaveData& rDst, const IslandSaveData& rSrc) {
    rDst.mFlags = rSrc.mFlags;
    rDst.mCompletedScenarios = rSrc.mCompletedScenarios;
    rDst.mStateBytes[0] = rSrc.mStateBytes[0];
    rDst.mStateBytes[1] = rSrc.mStateBytes[1];
    rDst.mCurActiveScenario = rSrc.mCurActiveScenario;
    __builtin_memcpy(rDst.mProgressBytes, rSrc.mProgressBytes, sizeof(rDst.mProgressBytes));
}

/**
 * @brief Copy persistent island progress between a record and its serialized form.
 * @param rDst Destination record.
 * @param rSrc Source record.
 */
template <typename TDst, typename TSrc>
ALWAYS_INLINE void copyRecord(TDst& rDst, const TSrc& rSrc) {
    rDst.mFlags = rSrc.mFlags;
    rDst.mCompletedScenarios = rSrc.mCompletedScenarios;
    rDst.mStateBytes[0] = rSrc.mStateBytes[0];
    rDst.mStateBytes[1] = rSrc.mStateBytes[1];
    __builtin_memcpy(rDst.mProgressBytes, rSrc.mProgressBytes, sizeof(rDst.mProgressBytes));
    rDst.mCurActiveScenario = rSrc.mCurActiveScenario;
}

} // namespace

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
        clearIsland(mpIslands[i]);
    }

    mpIslands[0].mFlags |= 1;
}

/**
 * @brief Clear the active vandalism flags of every island, keeping the recorded ones.
 */
void IslandSaveDataHolder::clearIslandVandalizedOnly() {
    for (int i = 0; i < mCount; ++i) {
        mpIslands[i].mStateBytes[2] = 0;
    }
}

/**
 * @brief Copy saved progress for all islands.
 * @param rOther Source holder with at least as many records as this holder.
 */
void IslandSaveDataHolder::copy(const IslandSaveDataHolder& rOther) {
    for (int i = 0; i < mCount; ++i) {
        copyIsland(mpIslands[i], rOther.mpIslands[i]);
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
 * @brief Record island progression.
 * @param islandId Island index within the allocated record count.
 */
void IslandSaveDataHolder::setIslandVisited(int islandId) { mpIslands[islandId].mFlags |= 1; }

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
    if (static_cast<u8>(rIsland.mStateBytes[1] & (1 << phase)) != 0) {
        *pActive = static_cast<u8>(rIsland.mStateBytes[2] & (1 << phase)) != 0;
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
    IslandSaveData& rIsland = mpIslands[islandId];
    rIsland.mStateBytes[1] |= 1 << phase;
    mpIslands[islandId].mStateBytes[2] |= 1 << phase;
}

/**
 * @brief Update both recorded and active vandalism flags.
 * @param islandId Island index within the allocated record count.
 * @param phase Phase bit from 0 through 7.
 */
void IslandSaveDataHolder::clearIslandVandalized(int islandId, int phase) {
    IslandSaveData& rIsland = mpIslands[islandId];
    rIsland.mStateBytes[1] &= ~(1 << phase);
    mpIslands[islandId].mStateBytes[2] &= ~(1 << phase);
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

/**
 * @brief Read island progress from a save stream.
 * @param pStream Non-null input stream positioned at the island block.
 * @return Always true; stream errors are handled by the stream.
 */
bool IslandSaveDataHolder::readFromStream(sead::ReadStream* pStream) {
    s32 count;
    pStream->readS32(count);
    IslandSaveRecord record = {};
    initialize();

    for (int i = 0; i < mCount; ++i) {
        pStream->readMemBlock(&record, sizeof(IslandSaveRecord));
        copyRecord(mpIslands[i], record);
    }

    return true;
}

/**
 * @brief Write island progress to a save stream.
 * @param pStream Non-null output stream receiving the island block.
 * @param isSkip True to only advance the stream past the records without writing them.
 */
void IslandSaveDataHolder::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    pStream->writeS32(mCount);
    if (isSkip) {
        for (int i = 0; i < mCount; ++i) {
            pStream->skip(sizeof(IslandSaveRecord));
        }

        return;
    }

    IslandSaveRecord record = {};

    for (int i = 0; i < mCount; ++i) {
        copyRecord(record, mpIslands[i]);
        pStream->writeMemBlock(&record, sizeof(IslandSaveRecord));
    }
}
