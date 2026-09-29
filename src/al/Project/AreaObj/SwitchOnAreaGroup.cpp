#include "Project/AreaObj/SwitchOnAreaGroup.hpp"
#include "Library/Scene/IScenarioCompleteChecker.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
    /**
     * @brief Constructs the group.
     * @param pAreaGroup The areas of the group.
     */
    SwitchOnAreaGroup::SwitchOnAreaGroup(AreaObjGroup* pAreaGroup) : mAreaGroup(pAreaGroup) {}

    /**
     * @brief Turns on the switch of every area that contains one of the points.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param isIgnoreDisasterCamera Whether areas with IsDisasterCameraOn are turned on as well.
     */
    void SwitchOnAreaGroup::update(const sead::Vector3f* pPoints, s32 numPoints, bool isIgnoreDisasterCamera) {
        s32 areaNum = mAreaGroup->getAreaObjCount();
        for (s32 i = 0; i < areaNum; i++) {
            AreaObj* areaObj = mAreaGroup->getAreaObj(i);
            if (isOnStageSwitch(areaObj, "SwitchAreaOn")) {
                continue;
            }

            for (s32 j = 0; j < numPoints; j++) {
                if (areaObj->isInVolume(pPoints[j]) && (isIgnoreDisasterCamera || !areaObj->mIsDisasterCameraOn)) {
                    onStageSwitch(areaObj, "SwitchAreaOn");
                    break;
                }
            }
        }
    }

    /**
     * @brief Turns on the switch of every area that contains a position.
     * @param rPos The position to check.
     */
    void SwitchOnAreaGroup::update(const sead::Vector3f& rPos) {
        sead::Vector3f pos = rPos;
        update(&pos, 1, false);
    }

    /**
     * @brief Disables the areas whose scenario has already been completed.
     * @param pChecker The checker for completed scenarios, or nullptr.
     */
    void SwitchOnAreaGroup::endInit(IScenarioCompleteChecker* pChecker) {
        if (pChecker == nullptr) {
            return;
        }

        s32 areaNum = mAreaGroup->getAreaObjCount();
        for (s32 i = 0; i < areaNum; i++) {
            AreaObj* areaObj = mAreaGroup->getAreaObj(i);
            if (areaObj->mScenarioID >= 0 && pChecker->isCompleteScenario(areaObj->mZoneID, areaObj->mScenarioID)) {
                areaObj->mIsDisabled = true;
            }
        }
    }

    /**
     * @brief Creates the group from the actor's linked switch-on areas.
     * @param pActor The actor the areas are linked to.
     * @param rInfo The actor's init info.
     * @return The created group, or nullptr if the actor has no linked areas.
     */
    SwitchOnAreaGroup* tryCreateSwitchOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo) {
        AreaObjGroup* areaGroup = createLinkAreaGroup(pActor, rInfo, "AreaSwitchOn", "子供スイッチOnエリアグループ", "子供スイッチOnエリア");
        if (areaGroup == nullptr) {
            return nullptr;
        }

        return new SwitchOnAreaGroup(areaGroup);
    }
};
