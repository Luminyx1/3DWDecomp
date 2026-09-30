#include "Project/Camera/Holder/CameraStopJudge.hpp"

#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {

CameraStopJudge::CameraStopJudge() = default;

bool CameraStopJudge::isStop() const {
    if (mIsInvalidStopJudgeByDemo) {
        return false;
    }
    return mIsInCameraStopArea || mIsStopByDeathPlayer;
}

void CameraStopJudge::update(const sead::Vector3f& rPos) {
    mIsInCameraStopArea = isInAreaObj(this, "CameraStopArea", rPos);
}

}  // namespace al
