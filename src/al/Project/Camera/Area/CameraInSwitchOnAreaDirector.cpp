#include "Project/Camera/Area/CameraInSwitchOnAreaDirector.hpp"

#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Camera/Core/CameraUtil.hpp"

namespace al {
/** @brief Creates a director without any areas. */
CameraInSwitchOnAreaDirector::CameraInSwitchOnAreaDirector() = default;

/**
 * @brief Sets the scene objects the director works with.
 * @param pSceneCameraInfo The scene's camera views.
 * @param pAreaObjDirector The scene's area director.
 */
void CameraInSwitchOnAreaDirector::init(const SceneCameraInfo* pSceneCameraInfo, AreaObjDirector* pAreaObjDirector) {
    mSceneCameraInfo = pSceneCameraInfo;
    mAreaObjDirector = pAreaObjDirector;
}

/** @brief Looks up the group of switch areas once all areas are placed. */
void CameraInSwitchOnAreaDirector::initAfterPlacement() {
    mAreaGroup = tryFindAreaObjGroup(this, "CameraInSwitchOnArea");
}

/** @brief Turns on the switch of each area that contains the look-at point of a valid camera view. */
void CameraInSwitchOnAreaDirector::update() {
    if (mAreaGroup == nullptr) {
        return;
    }

    s32 areaNum = mAreaGroup->mNumAreas;
    for (s32 i = 0; i < areaNum; i++) {
        AreaObj* area = mAreaGroup->getAreaObj(i);
        s32 viewNum = getViewNumMax(mSceneCameraInfo);
        for (s32 j = 0; j < viewNum; j++) {
            if (isValidView(mSceneCameraInfo, j) && isInAreaPos(area, getCameraAt_RS(mSceneCameraInfo, j))) {
                tryOnStageSwitch(area, "SwitchCameraInOn");
                break;
            }
        }
    }
}
}  // namespace al
