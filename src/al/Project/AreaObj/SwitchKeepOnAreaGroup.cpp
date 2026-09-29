#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
    /**
     * @brief Constructs the group.
     * @param pAreaGroup The areas of the group.
     */
    SwitchKeepOnAreaGroup::SwitchKeepOnAreaGroup(AreaObjGroup* pAreaGroup)
        : mAreaGroup(pAreaGroup), mOnAreas(nullptr), mMaxOnAreas(0), mNumOnAreas(0) {
        mMaxOnAreas = pAreaGroup->getAreaObjCount();
        if (mMaxOnAreas > 0) {
            mOnAreas = new AreaObj*[mMaxOnAreas];
        }
    }

    /**
     * @brief Turns on the switch of the areas that contain the points and turns off the others.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param isIgnoreDisasterCamera Whether areas with IsDisasterCameraOn are turned on as well.
     */
    void SwitchKeepOnAreaGroup::update(const sead::Vector3f* pPoints, s32 numPoints, bool isIgnoreDisasterCamera) {
        mNumOnAreas = 0;
        s32 areaNum = mAreaGroup->getAreaObjCount();
        for (s32 i = 0; i < areaNum; i++) {
            AreaObj* areaObj = mAreaGroup->getAreaObj(i);

            bool isSameSwitchOn = false;
            for (s32 j = 0; j < mNumOnAreas; j++) {
                if (isSameStageSwitch(areaObj, mOnAreas[j], "SwitchAreaOn")) {
                    isSameSwitchOn = true;
                    break;
                }
            }
            if (isSameSwitchOn) {
                continue;
            }

            s32 onCondition = 0;
            tryGetArg(&onCondition, *areaObj->mPlacementInfo, "OnCondition");

            bool isIn = false;
            if (onCondition == 0) {
                for (s32 j = 0; j < numPoints; j++) {
                    if (areaObj->isInVolume(pPoints[j])) {
                        isIn = true;
                        break;
                    }
                }
            } else if (onCondition == 1) {
                for (s32 j = 0; j < numPoints; j++) {
                    if (!areaObj->isInVolume(pPoints[j])) {
                        isIn = false;
                        break;
                    }
                    isIn = true;
                }
            }

            if (!isIn) {
                tryOffStageSwitch(areaObj, "SwitchAreaOn");
                continue;
            }

            if (areaObj->mIsDisasterCameraOn && !isIgnoreDisasterCamera) {
                continue;
            }

            tryOnStageSwitch(areaObj, "SwitchAreaOn");
            mOnAreas[mNumOnAreas] = areaObj;
            mNumOnAreas++;
        }
    }

    /**
     * @brief Turns on the switch of the areas that contain a position and turns off the others.
     * @param rPos The position to check.
     */
    void SwitchKeepOnAreaGroup::update(const sead::Vector3f& rPos) {
        sead::Vector3f pos = rPos;
        update(&pos, 1, false);
    }

    /**
     * @brief Creates the group from the actor's linked switch-keep-on areas.
     * @param pActor The actor the areas are linked to.
     * @param rInfo The actor's init info.
     * @return The created group, or nullptr if the actor has no linked areas.
     */
    SwitchKeepOnAreaGroup* tryCreateSwitchKeepOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo) {
        AreaObjGroup* areaGroup = createLinkAreaGroup(pActor, rInfo, "AreaSwitchKeepOn", "子供スイッチキープエリアグループ", "子供スイッチキープエリア");
        if (areaGroup == nullptr) {
            return nullptr;
        }

        return new SwitchKeepOnAreaGroup(areaGroup);
    }
};
