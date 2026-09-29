#include "Project/Camera/Area/AreaCameraSwitchInfo.hpp"

#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/** @brief Creates an info that is not bound to any area yet. */
AreaCameraSwitchInfo::AreaCameraSwitchInfo() = default;

/**
 * @brief Binds the info to an area and reads the switch flags from its arguments.
 * @param pArea The camera area.
 * @param priority The priority of the area's camera.
 * @param isInterpoleIn Whether switching to the camera interpolates.
 * @param isInterpoleOut Whether switching away from the camera interpolates.
 */
void AreaCameraSwitchInfo::initArea(const AreaObj* pArea, s32 priority, bool isInterpoleIn, bool isInterpoleOut) {
    mArea = pArea;
    mPriority = priority;
    mIsInterpoleIn = isInterpoleIn;
    mIsInterpoleOut = isInterpoleOut;
    tryGetAreaObjArg(&mIsEndNoInterpole, mArea, "IsEndNoInterpole");
    tryGetAreaObjArg(&mIsStartWhenCollideGround, mArea, "IsStartWhenCollideGround");
    tryGetAreaObjArg(&mIsStartWhenInWater, mArea, "IsStartWhenInWater");
}
}  // namespace al
