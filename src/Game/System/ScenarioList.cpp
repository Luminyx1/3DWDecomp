#include "System/ScenarioList.hpp"

/**
 * @brief Allocates the requested number of empty scenario records.
 * @param count Nonnegative number of scenarios to allocate.
 */
ScenarioList::ScenarioList(s32 count) : mScenarios(new ScenarioData[count]), mCount(count) {}

/**
 * @brief Gets a scenario, clamping an index beyond the end to the last entry.
 * @param index Nonnegative scenario index; the list must contain at least one entry.
 * @return Selected scenario record.
 */
ScenarioData* ScenarioList::getScenarioDataByIndex(s32 index) {
    if (index >= mCount) {
        index = mCount - 1;
    }
    return &mScenarios[index];
}

/**
 * @brief Creates an unassigned scenario with an empty display name.
 */
ScenarioData::ScenarioData() {}

/**
 * @brief Checks whether the scenario uses the main-scenario classification.
 * @return True for the main-scenario type.
 */
bool ScenarioData::isMainScenario() const { return mScenarioType == 1; }

/**
 * @brief Checks whether the scenario type is greater than the main-scenario type.
 * @return True for types above one, including the unassigned sentinel.
 */
bool ScenarioData::isSpecialScenario() const { return mScenarioType > 1; }
