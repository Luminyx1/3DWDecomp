#include "System/Data/OceanScenarioList.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Scene/SceneObjID.hpp"

/**
 * @brief Finds a quadrant scenario list through the ocean scenario list scene object.
 * @param pUser Non-null scene user used to locate the ocean list.
 * @param quadrant Quadrant index; invalid indices return nullptr.
 * @return Scenario list, or nullptr when unavailable.
 */
ScenarioList* OceanScenarioList::tryGetOceanScenarioList(const al::IUseSceneObjHolder* pUser, s32 quadrant) {
    auto* pLists = al::tryGetSceneObj<OceanScenarioList>(pUser, SceneObjID_OceanScenarioList);
    return pLists != nullptr ? pLists->getScenarioListByQuadrant(quadrant) : nullptr;
}

/**
 * @brief Looks up a quadrant scenario list.
 * @param quadrant Zero-based quadrant index; negative and out-of-range values return nullptr.
 * @return Matching list, or nullptr.
 */
ScenarioList* OceanScenarioList::getScenarioListByQuadrant(s32 quadrant) {
    if (static_cast<u32>(quadrant) < static_cast<u32>(mLists.size())) {
        return mLists[quadrant];
    }
    return nullptr;
}

/**
 * @brief Creates an empty set of quadrant scenario lists.
 * @param capacity Maximum number of quadrant lists that can be added.
 */
OceanScenarioList::OceanScenarioList(s32 capacity) {
    mLists.allocBuffer(capacity, nullptr);
}

/**
 * @brief Appends a quadrant list when capacity is available.
 * @param pList Scenario list to append; subsequent queries require a non-null list.
 */
void OceanScenarioList::addList(ScenarioList* pList) {
    mLists.pushBack(pList);
}

/**
 * @brief Counts scenarios across all quadrant lists.
 * @return Total number of scenarios; stored lists must be valid.
 */
s32 OceanScenarioList::getScenarioNum() const {
    s32 count = 0;
    for (s32 i = 0; i < mLists.size(); ++i) {
        count += mLists[i]->mCount;
    }
    return count;
}

/**
 * @brief Finds the first quadrant containing a scenario identifier.
 * @param scenarioId Scenario identifier to search for; stored lists must be valid.
 * @return First matching quadrant, or -1 if absent.
 */
s32 OceanScenarioList::getQuadrantIndexFromScenarioId(s32 scenarioId) const {
    for (s32 i = 0; i < mLists.size(); ++i) {
        const ScenarioList* pList = mLists[i];
        for (s32 j = 0; j < pList->mCount; ++j) {
            if (pList->mScenarios[j].mScenarioId == scenarioId) {
                return i;
            }
        }
    }
    return -1;
}
