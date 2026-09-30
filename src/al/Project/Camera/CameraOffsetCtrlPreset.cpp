#include "Project/Camera/CameraOffsetCtrlPreset.hpp"

#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Camera/CameraOffsetPreset.hpp"

namespace al {

void CameraOffsetCtrlY::load(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffset.y, rIter, "OffsetY");
}

CameraOffsetCtrlPreset::CameraOffsetCtrlPreset() {
    mPreset = new CameraOffsetPreset();
}

void CameraOffsetCtrlPreset::load(const ByamlIter& rIter) {
    mPreset->loadParam(rIter);
}

const sead::Vector3f& CameraOffsetCtrlPreset::getOffset() const {
    return mPreset->getOffset();
}

}  // namespace al
