#include "System/IslandSaveData.hpp"
#include <prim/seadBitFlag.h>

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
