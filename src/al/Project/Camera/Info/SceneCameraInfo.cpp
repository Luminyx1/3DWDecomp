#include "Project/Camera/Info/SceneCameraInfo.hpp"

#include "Project/Camera/Info/CameraViewInfo.hpp"

namespace al {
/**
 * @brief Creates the info with room for the given number of views, all empty.
 * @param viewNumMax The maximum number of camera views.
 */
SceneCameraInfo::SceneCameraInfo(s32 viewNumMax) : mViewNumMax(viewNumMax) {
    mViewArray = new CameraViewInfo*[viewNumMax];
    for (s32 i = 0; i < mViewNumMax; i++) {
        mViewArray[i] = nullptr;
    }
}

/**
 * @brief Registers a view in the slot of its index.
 * @param pViewInfo The view to register.
 */
void SceneCameraInfo::initViewInfo(CameraViewInfo* pViewInfo) {
    mViewArray[pViewInfo->mIndex] = pViewInfo;
}

/**
 * @brief Gets the display name of a view.
 * @param index The index of the view.
 * @return "メイン" (main) for the first view, "サブ" (sub) for the second one and "TV" otherwise.
 */
const char* SceneCameraInfo::getViewName(s32 index) const {
    if (index == 0) {
        return "メイン";
    }
    else if (index == 1) {
        return "サブ";
    }
    else {
        return "TV";
    }
}
}  // namespace al
