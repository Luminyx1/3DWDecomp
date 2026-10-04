#include "System/IslandSaveDataHolder.hpp"
#include <prim/seadBitFlag.h>
#include <stream/seadStream.h>

/**
 * @brief Creates an empty island save record.
 */
IslandSaveData::IslandSaveData() { initialize(); }

/**
 * @brief Clears island progress and selects scenario zero.
 */
void IslandSaveData::initialize() {
    mFlags = 0;
    mCompletedScenarios = 0;
    for (auto& rByte : mStateBytes) {
        rByte = 0;
    }
    for (auto& rByte : mProgressBytes) {
        rByte = 0;
    }
    mCurActiveScenario = 0;
}

/**
 * @brief Copies persistent island progress, retaining the third transient state byte.
 * @param rOther Source island record; self-copy is permitted.
 */
void IslandSaveData::copy(const IslandSaveData& rOther) {
    mFlags = rOther.mFlags;
    mCompletedScenarios = rOther.mCompletedScenarios;
    mStateBytes[0] = rOther.mStateBytes[0];
    mStateBytes[1] = rOther.mStateBytes[1];
    mCurActiveScenario = rOther.mCurActiveScenario;
    __builtin_memcpy(mProgressBytes, rOther.mProgressBytes, sizeof(mProgressBytes));
}

/**
 * @brief Selects the active scenario and clears its name-seen flag when it changes.
 * @param scenarioId New active scenario identifier; stored without validation.
 */
void IslandSaveData::setCurActiveScenario(s32 scenarioId) {
    if (mCurActiveScenario != scenarioId) {
        mFlags &= ~4;
    }
    mCurActiveScenario = scenarioId;
}

/**
 * @brief Marks the active scenario name as seen.
 */
void IslandSaveData::setCurActiveScenarioNameSeen() { mFlags |= 4; }

/**
 * @brief Checks whether the active scenario name was shown.
 * @return True when the name-seen flag is set.
 */
bool IslandSaveData::wasActiveScenarioNameSeen() const { return (mFlags & 4) != 0; }

/**
 * @brief Checks whether a scenario is complete.
 * @param scenarioId Scenario bit index from 0 to 63; not bounds-checked.
 * @return True when its completion flag is set.
 */
bool IslandSaveData::isScenarioComplete(s32 scenarioId) const {
    const u64 mask = 1ull << static_cast<u32>(scenarioId);
    return (mCompletedScenarios & mask) != 0;
}

/**
 * @brief Records completion of a scenario.
 * @param scenarioId Scenario bit index from 0 to 63; not bounds-checked.
 */
void IslandSaveData::completeScenario(s32 scenarioId) {
    const u64 mask = 1ull << static_cast<u32>(scenarioId);
    mCompletedScenarios |= mask;
}

/**
 * @brief Clears completion of a scenario.
 * @param scenarioId Scenario bit index from 0 to 63; not bounds-checked.
 */
void IslandSaveData::resetScenario(s32 scenarioId) {
    const u64 mask = 1ull << static_cast<u32>(scenarioId);
    mCompletedScenarios &= ~mask;
}

/**
 * @brief Counts completed scenarios.
 * @return Number of set completion flags.
 */
s32 IslandSaveData::getNumScenariosComplete() const {
    return sead::BitFlagUtil::countOnBit64(mCompletedScenarios);
}

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
 * @brief Copy persistent island progress between a record and its serialized form.
 * @param rDst Destination record.
 * @param rSrc Source record.
 */
template <typename TDst, typename TSrc>
void copyRecord(TDst& rDst, const TSrc& rSrc) {
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
        mpIslands[i].initialize();
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
