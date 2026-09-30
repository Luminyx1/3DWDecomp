#include "Project/AreaObj/SwitchOnAreaGroup.hpp"

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/Scene/IScenarioCompleteChecker.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
/**
 * Constructs a switch on group for an area group.
 * @param pGroup area group
 */
SwitchOnAreaGroup::SwitchOnAreaGroup(AreaObjGroup* pGroup) : mGroup(pGroup) {}

/**
 * Turns on the switches of the areas containing any of the positions.
 * @param pPositions positions to check
 * @param num number of positions
 * @param isDisasterMode whether disaster mode is active
 */
void SwitchOnAreaGroup::update(const sead::Vector3f* pPositions, s32 num, bool isDisasterMode) {
    s32 numAreas = mGroup->mNumAreas;
    for (s32 i = 0; i < numAreas; i++) {
        AreaObj* areaObj = mGroup->getAreaObj(i);
        if (isOnStageSwitch(areaObj, "SwitchAreaOn")) {
            continue;
        }

        for (s32 j = 0; j < num; j++) {
            if (areaObj->isInVolume(pPositions[j]) &&
                (isDisasterMode || !areaObj->mIsDisasterCameraOn)) {
                onStageSwitch(areaObj, "SwitchAreaOn");
                break;
            }
        }
    }
}

/**
 * Turns on the switches of the areas containing a position.
 * @param rPos position to check
 */
void SwitchOnAreaGroup::update(const sead::Vector3f& rPos) {
    sead::Vector3f pos = rPos;
    update(&pos, 1, false);
}

/**
 * Disables the areas whose scenario is already complete.
 * @param pChecker scenario completion checker
 */
void SwitchOnAreaGroup::endInit(IScenarioCompleteChecker* pChecker) {
    if (!pChecker) {
        return;
    }

    s32 numAreas = mGroup->mNumAreas;
    for (s32 i = 0; i < numAreas; i++) {
        AreaObj* areaObj = mGroup->getAreaObj(i);
        if (areaObj->mScenarioID < 0) {
            continue;
        }

        if (pChecker->isScenarioComplete(areaObj->mZoneID, areaObj->mScenarioID)) {
            areaObj->disable();
        }
    }
}

/**
 * Creates a switch on group from the areas linked to an actor.
 * @param pActor actor the areas are linked to
 * @param rInfo actor init info
 * @return the group, or nullptr if no areas are linked
 */
SwitchOnAreaGroup* tryCreateSwitchOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo) {
    AreaObjGroup* group = createLinkAreaGroup(pActor, rInfo, "AreaSwitchOn",
                                              "子供スイッチOnエリアグループ",
                                              "子供スイッチOnエリア");
    if (!group) {
        return nullptr;
    }

    return new SwitchOnAreaGroup(group);
}
}  // namespace al
