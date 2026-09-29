#include "Project/Camera/Holder/CameraStopJudge.hpp"

#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/** @brief Creates a judge that does not stop the camera. */
CameraStopJudge::CameraStopJudge() = default;

/**
 * @brief Checks whether the camera should currently be stopped.
 * @return True if the camera is stopped by an area or a request, unless a demo disabled the judge.
 */
bool CameraStopJudge::isStop() const {
    if (mIsInvalidStopJudgeByDemo) {
        return false;
    }
    return mIsInCameraStopArea || _9;
}

/**
 * @brief Checks whether the given position is inside a camera stop area.
 * @param rPos The position of the camera target.
 */
void CameraStopJudge::update(const sead::Vector3f& rPos) {
    mIsInCameraStopArea = isInAreaObj(this, "CameraStopArea", rPos);
}
}  // namespace al
