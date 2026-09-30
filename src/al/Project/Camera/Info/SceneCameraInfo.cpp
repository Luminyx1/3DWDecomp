#include "Project/Camera/Info/SceneCameraInfo.hpp"

#include "Project/Camera/Info/CameraViewInfo.hpp"

namespace al {

SceneCameraInfo::SceneCameraInfo(s32 viewNum) : mViewNumMax(viewNum) {
    mViewArray = new CameraViewInfo*[viewNum];
    for (s32 i = 0; i < mViewNumMax; i++) {
        mViewArray[i] = nullptr;
    }
}

void SceneCameraInfo::initViewInfo(CameraViewInfo* pViewInfo) {
    mViewArray[pViewInfo->getIndex()] = pViewInfo;
}

const char* SceneCameraInfo::getViewName(s32 index) const {
    if (index == 0) {
        return "メイン";
    }

    if (index == 1) {
        return "サブ";
    }

    return "TV";
}

}  // namespace al
