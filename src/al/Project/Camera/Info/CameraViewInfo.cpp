#include "Project/Camera/Info/CameraViewInfo.hpp"

#include "Library/Projection/Projection.hpp"

namespace al {

CameraViewInfo::CameraViewInfo(s32 index, const sead::LookAtCamera& rCamera,
                               Projection& rProjection, const CameraViewFlag& rFlag,
                               const OrthoProjectionInfo& rOrthoInfo)
    : mIndex(index), mLookAtCam(rCamera), mProjection(rProjection), mFlag(rFlag),
      mOrthoProjectionInfo(rOrthoInfo) {}

sead::Projection& CameraViewInfo::getProjectionSead() {
    return mProjection.getProjectionSead();
}

const sead::Projection& CameraViewInfo::getProjectionSead() const {
    return mProjection.getProjectionSead();
}

const sead::Matrix44f* CameraViewInfo::getProjMtx() const {
    return &mProjection.getProjMtx();
}

f32 CameraViewInfo::getAspect() const {
    return mProjection.getAspect();
}

f32 CameraViewInfo::getNear() const {
    return mProjection.getNear();
}

f32 CameraViewInfo::getFar() const {
    return mProjection.getFar();
}

}  // namespace al
