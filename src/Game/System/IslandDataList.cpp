#include "System/IslandDataList.hpp"

#include "Project/Message/MessageTagUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Library/Stage/StageInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "System/Data/OceanScenarioList.hpp"
#include "System/IslandData.hpp"

/**
 * @brief Creates an unassigned island with no scenario list.
 */
IslandData::IslandData() : mDisplayName(u"") {}

/**
 * @brief Fills an island record and allocates its scenario list.
 * @param pName Internal island name.
 * @param pDisplayName Display name, converted to a wide string.
 * @param islandId Identifier assigned to the island.
 * @param unlockCount Unlock requirement of the island.
 * @param scenarioNum Number of scenarios to allocate.
 */
void IslandData::init(const char* pName, const char* pDisplayName, s32 islandId, s32 unlockCount,
                      s32 scenarioNum) {
    mName = pName;
    mIslandId = islandId;
    mAttribute = unlockCount;
    mDisplayName.convertFromMultiByteString(pDisplayName, -1);
    mScenarios = new ScenarioList(scenarioNum);
}

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

/**
 * @brief Creates an empty list with room for a fixed number of islands.
 * @param capacity Number of island records to allocate.
 */
IslandDataList::IslandDataList(int capacity) : mNum(0), mCapacity(capacity) {
    mpIslands = new IslandData[capacity];
}

/**
 * @brief Creates the list from the system island BYAML.
 */
IslandDataList::IslandDataList() {
    al::Resource* pResource = al::findOrCreateResource("SystemData/IslandList", nullptr);
    al::ByamlIter rootIter(pResource->getByml("IslandList"));
    al::ByamlIter islandIter;
    al::ByamlIter listIter;
    rootIter.tryGetIterByKey(&listIter, "IslandList");
    mNum = mCapacity = listIter.getSize();
    mpIslands = new IslandData[mCapacity];

    for (int i = 0; i < mNum; i++) {
        listIter.tryGetIterByIndex(&islandIter, i);
        int islandId = -1;
        islandIter.tryGetIntByKey(&islandId, "IslandID");
        SharedIslandInit(&mpIslands[i], islandIter, islandId);
    }
}

/**
 * @brief Initializes an island record and its scenarios from BYAML.
 * @param pIsland Island record to fill.
 * @param rIter Iterator over the island's BYAML dictionary.
 * @param islandId Identifier assigned to the island.
 * @return Always true.
 */
bool IslandDataList::SharedIslandInit(IslandData* pIsland, al::ByamlIter& rIter, int islandId) {
    al::ByamlIter scenarioIter;
    al::ByamlIter scenarioListIter;
    const char* pName = "";
    const char* pDisplayName = "";
    const char* pStageName;
    const char* pScenarioDisplayName = "";
    const char* pType;
    int unlockCount = -1;
    int scenarioId;
    int unlockScenarioId;
    bool isDisasterDisable;
    bool isUpdateLevelData;
    rIter.tryGetStringByKey(&pName, "IslandName");
    rIter.tryGetStringByKey(&pDisplayName, "IslandDisplayName");
    rIter.tryGetIntByKey(&unlockCount, "UnlockCount");
    rIter.tryGetIterByKey(&scenarioListIter, "ScenarioList");
    int scenarioNum = scenarioListIter.getSize();

    pIsland->init(pName, pDisplayName, islandId, unlockCount, scenarioNum);

    for (int i = 0; i < scenarioNum; i++) {
        scenarioListIter.tryGetIterByIndex(&scenarioIter, i);
        scenarioId = -1;
        pStageName = "MissingName";
        pType = "InvalidType";
        pScenarioDisplayName = "";
        unlockScenarioId = -1;
        isDisasterDisable = false;
        isUpdateLevelData = false;
        scenarioIter.tryGetIntByKey(&scenarioId, "ScenarioID");
        scenarioIter.tryGetStringByKey(&pStageName, "ScenarioName");
        scenarioIter.tryGetStringByKey(&pScenarioDisplayName, "ScenarioDisplayName");
        scenarioIter.tryGetStringByKey(&pType, "ScenarioType");
        scenarioIter.tryGetIntByKey(&unlockScenarioId, "UnlockScenarioID");
        scenarioIter.tryGetBoolByKey(&isDisasterDisable, "DisasterDisable");
        scenarioIter.tryGetBoolByKey(&isUpdateLevelData, "UpdateLevelData");
        pIsland->mScenarios->getScenarioDataByIndex(i)->init(
            scenarioId, pStageName, pScenarioDisplayName, pType, unlockScenarioId,
            isUpdateLevelData, isDisasterDisable);
    }

    return true;
}

/**
 * @brief Fills an unused island slot from a stage's island BYAML.
 * @param pStageInfo Stage whose resource holds the island BYAML.
 * @param islandId One-based island identifier selecting the slot.
 * @return True if the slot was empty and the BYAML existed.
 */
bool IslandDataList::RegisterIslandData(const al::StageInfo* pStageInfo, int islandId) {
    if (islandId < 1 || islandId >= mCapacity) {
        return false;
    }

    IslandData* pIsland = &mpIslands[islandId - 1];
    if (pIsland->mName != nullptr) {
        return false;
    }

    const u8* pByml = pStageInfo->getResource()->tryGetByml("Island");
    if (pByml == nullptr) {
        return false;
    }

    al::ByamlIter iter(pByml);
    SharedIslandInit(pIsland, iter, islandId);
    return true;
}

/**
 * @brief Gets an island record by index.
 * @param index Zero-based island index.
 * @return The island, or nullptr when the index is out of range.
 */
IslandData* IslandDataList::getIslandByIndex(int index) {
    if (index < 0 || index >= mNum) {
        return nullptr;
    }

    return &mpIslands[index];
}

/**
 * @brief Trims the island count to the last registered island.
 */
void IslandDataList::endInit() {
    int num = mCapacity;
    while (num > 0 && mpIslands[num - 1].mName == nullptr) {
        num--;
    }

    mNum = num;
}

/**
 * @brief Looks up the localized name of an island.
 * @param pHolder Scene object holder (unused).
 * @param pMessageSystem Message system holding the system messages.
 * @param islandId Island identifier.
 * @return The localized island name.
 */
const char* IslandDataFunction::getIslandName(al::IUseSceneObjHolder* pHolder,
                                              al::IUseMessageSystem* pMessageSystem,
                                              int islandId) {
    al::StringTmp<32> label("Island%d_Name", islandId);
    return al::getSystemMessageString(pMessageSystem, "IslandName", label.cstr());
}

/**
 * @brief Looks up the localized name of an island scenario.
 * @param pHolder Scene object holder (unused).
 * @param pMessageSystem Message system holding the system messages.
 * @param islandId Island identifier.
 * @param scenarioId Scenario number on the island.
 * @return The localized scenario name.
 */
const char* IslandDataFunction::getIslandScenarioName(al::IUseSceneObjHolder* pHolder,
                                                      al::IUseMessageSystem* pMessageSystem,
                                                      int islandId, int scenarioId) {
    al::StringTmp<32> label("Island%d_Scenario%d", islandId, scenarioId);
    return al::getSystemMessageString(pMessageSystem, "ScenarioName", label.cstr());
}

/**
 * @brief Looks up the localized name of an ocean quadrant scenario.
 * @param pHolder Scene object holder providing the ocean scenario list.
 * @param pMessageSystem Message system holding the system messages.
 * @param scenarioId One-based scenario number in the quadrant.
 * @param quadrant Quadrant index.
 * @return The localized scenario name, or an empty string without an ocean scenario list.
 */
const char* IslandDataFunction::getOceanScenarioName(al::IUseSceneObjHolder* pHolder,
                                                     al::IUseMessageSystem* pMessageSystem,
                                                     int scenarioId, int quadrant) {
    if (al::tryGetSceneObj(pHolder, 42) == nullptr) {
        return "";
    }

    OceanScenarioList::tryGetOceanScenarioList(pHolder, quadrant)
        ->getScenarioDataByIndex(scenarioId - 1);
    const char* pQuadrantName = getQuadrantNameFromQuadrantIndex(quadrant);
    al::StringTmp<32> label("Ocean_");
    label.append(pQuadrantName);
    label.appendWithFormat("_Scenario%d", scenarioId);
    return al::getSystemMessageString(pMessageSystem, "ScenarioName", label.cstr());
}

/**
 * @brief Gets the compass name of an ocean quadrant.
 * @param quadrant Quadrant index from 0 through 3.
 * @return The quadrant name, or an empty string for other values.
 */
const char* IslandDataFunction::getQuadrantNameFromQuadrantIndex(int quadrant) {
    static const char* const sQuadrantNames[] = {"South", "East", "North", "West"};

    if (static_cast<unsigned int>(quadrant) <= 3) {
        return sQuadrantNames[quadrant];
    }
    return "";
}

/**
 * @brief Converts an ocean pseudo-island identifier to a quadrant index.
 * @param islandId Island identifier; ocean quadrants use -1 through -4.
 * @return Quadrant index, defaulting to zero for ordinary islands.
 */
int IslandDataFunction::getQuadrantIndexFromIslandID(int islandId) {
    switch (islandId) {
    case -1:
        return 0;
    case -2:
        return 1;
    case -3:
        return 2;
    case -4:
        return 3;
    default:
        return 0;
    }
}

/**
 * @brief Converts a quadrant index to its ocean pseudo-island identifier.
 * @param quadrant Quadrant index from 0 through 3; other values return zero.
 * @return The negative ocean pseudo-island identifier.
 */
int IslandDataFunction::getIslandIDFromQuadrantIndex(int quadrant) {
    switch (quadrant) {
    case 0:
        return -1;
    case 1:
        return -2;
    case 2:
        return -3;
    case 3:
        return -4;
    default:
        return 0;
    }
}

/**
 * @brief Converts a placement parameter to an ocean pseudo-island identifier.
 * @param value Placement parameter from 1 through 4; other values return zero.
 * @return The negative pseudo-island identifier.
 */
int IslandDataFunction::getIslandIDFromParam(int value) {
    switch (value) {
    case 1:
        return -1;
    case 2:
        return -2;
    case 3:
        return -3;
    case 4:
        return -4;
    default:
        return 0;
    }
}

/**
 * @brief Converts a placement parameter to a quadrant index.
 * @param value Placement parameter selecting one of four quadrants.
 * @return The zero-based quadrant index.
 */
int IslandDataFunction::getQuadrantIndexFromParam(int value) {
    switch (value) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    default:
        return 0;
    }
}

/**
 * @brief Checks whether an island is one of the Giga Bell islands.
 * @param islandId Island identifier to test.
 * @return True for island identifiers 14 through 16.
 */
bool IslandDataFunction::isGigaBellIsland(int islandId) {
    return static_cast<unsigned int>(islandId - 14) < 3;
}
