#include "System/IslandData.hpp"

/**
 * @brief Creates an unassigned island with no scenario list.
 */
IslandData::IslandData() {}

/**
 * @brief Gets a scenario from the island list.
 * @param index Nonnegative scenario index; the island must have a nonempty list.
 * @return Selected scenario, clamped at the upper end of the list.
 */
ScenarioData* IslandData::getScenarioDataByIndex(s32 index) {
    return mScenarios->getScenarioDataByIndex(index);
}

/**
 * @brief Gets the number of scenarios on an initialized island.
 * @return Scenario count; requires a valid scenario list.
 */
s32 IslandData::getNumScenarios() const { return mScenarios->mCount; }

/**
 * @brief Checks whether an island has a scenario list.
 * @return True when the list pointer is non-null.
 */
bool IslandData::isValid() const { return mScenarios != nullptr; }
