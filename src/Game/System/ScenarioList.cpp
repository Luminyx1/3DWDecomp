#include "System/ScenarioList.hpp"

#include "Project/Base/StringUtil.hpp"

namespace {

constexpr u32 cScenarioTypeNormal = 0;
constexpr u32 cScenarioTypeMain = 1;
constexpr u32 cScenarioTypeRabbit = 2;
constexpr u32 cScenarioTypeCloud = 3;
constexpr u32 cScenarioTypeRally = 4;
constexpr u32 cScenarioTypeBrigade = 5;
constexpr u32 cScenarioTypeNeko = 6;
constexpr u32 cScenarioTypeMystery = 7;
constexpr u32 cScenarioTypePlessie = 8;
constexpr u32 cScenarioTypeLucky = 9;
constexpr u32 cScenarioTypeInvalid = static_cast<u32>(-1);

}  // namespace

/**
 * @brief Allocates the requested number of empty scenario records.
 * @param count Nonnegative number of scenarios to allocate.
 */
ScenarioList::ScenarioList(s32 count) {
    mCount = count;
    mScenarios = new ScenarioData[count];
}

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
ScenarioData::ScenarioData() : mName(u"") {}

/**
 * @brief Fills a scenario record from its list entry.
 * @param scenarioId Identifier assigned to the scenario.
 * @param pStageName Stage name; the pointer is stored, not copied.
 * @param pDisplayName Display name, converted to a wide string.
 * @param pType Scenario type name; unknown names map to the unassigned type.
 * @param unlockScenarioId Scenario that unlocks this one.
 * @param isUpdateLevelData Whether clearing the scenario updates level data.
 * @param isDisasterDisable Whether disasters are disabled in the scenario.
 */
void ScenarioData::init(s32 scenarioId, const char* pStageName, const char* pDisplayName,
                        const char* pType, s32 unlockScenarioId, bool isUpdateLevelData,
                        bool isDisasterDisable) {
    mScenarioId = scenarioId;
    mStageName = pStageName;
    mName.convertFromMultiByteString(pDisplayName, -1);

    u32 type;
    if (al::isEqualString(pType, "Normal")) {
        type = cScenarioTypeNormal;
    } else if (al::isEqualString(pType, "Main")) {
        type = cScenarioTypeMain;
    } else if (al::isEqualString(pType, "Rabbit")) {
        type = cScenarioTypeRabbit;
    } else if (al::isEqualString(pType, "Cloud")) {
        type = cScenarioTypeCloud;
    } else if (al::isEqualString(pType, "Rally")) {
        type = cScenarioTypeRally;
    } else if (al::isEqualString(pType, "Brigade")) {
        type = cScenarioTypeBrigade;
    } else if (al::isEqualString(pType, "Neko")) {
        type = cScenarioTypeNeko;
    } else if (al::isEqualString(pType, "Mystery")) {
        type = cScenarioTypeMystery;
    } else if (al::isEqualString(pType, "Plessie")) {
        type = cScenarioTypePlessie;
    } else if (al::isEqualString(pType, "Lucky")) {
        type = cScenarioTypeLucky;
    } else {
        type = cScenarioTypeInvalid;
    }

    mScenarioType = type;
    mAttribute = unlockScenarioId;
    mFlag130 = isUpdateLevelData;
    mFlag131 = isDisasterDisable;
}

/**
 * @brief Checks whether the scenario uses the main-scenario classification.
 * @return True for the main-scenario type.
 */
bool ScenarioData::isMainScenario() const { return mScenarioType == cScenarioTypeMain; }

/**
 * @brief Checks whether the scenario type is greater than the main-scenario type.
 * @return True for types above one, including the unassigned sentinel.
 */
bool ScenarioData::isSpecialScenario() const { return mScenarioType > cScenarioTypeMain; }
