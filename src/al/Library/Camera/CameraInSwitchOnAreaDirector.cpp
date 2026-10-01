#include "Library/Camera/CameraInSwitchOnAreaDirector.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {

CameraInSwitchOnAreaDirector::CameraInSwitchOnAreaDirector() = default;

void CameraInSwitchOnAreaDirector::init(const SceneCameraInfo* pSceneCameraInfo,
                                        AreaObjDirector* pAreaObjDirector) {
    mSceneCameraInfo = pSceneCameraInfo;
    mAreaObjDirector = pAreaObjDirector;
}

void CameraInSwitchOnAreaDirector::initAfterPlacement() {
    mAreaObjGroup = tryFindAreaObjGroup(this, "CameraInSwitchOnArea");
}

void CameraInSwitchOnAreaDirector::update() {
    if (mAreaObjGroup == nullptr) {
        return;
    }

    s32 areaNum = mAreaObjGroup->getSize();

    for (s32 i = 0; i < areaNum; i++) {
        AreaObj* area = mAreaObjGroup->getAreaObj(i);
        s32 viewNum = getViewNumMax(mSceneCameraInfo);

        for (s32 j = 0; j < viewNum; j++) {
            if (!isValidView(mSceneCameraInfo, j)) {
                continue;
            }

            if (isInAreaPos(area, getCameraAt_RS(mSceneCameraInfo, j))) {
                tryOnStageSwitch(area, "SwitchCameraInOn");
                break;
            }
        }
    }
}

}  // namespace al
