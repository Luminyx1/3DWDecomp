#include "Library/Camera/CameraPoserFlag.hpp"

#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Creates the flags with their default values.
 */
CameraPoserFlag::CameraPoserFlag() = default;

/**
 * Loads the flags from a camera parameter.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserFlag::load(const ByamlIter& rIter) {
    tryGetByamlBool(&isInvalidChangeSubjective, rIter, "IsInvalidChangeSubjective");
    tryGetByamlBool(&isValidKeepPreSelfPoseNextCameraByParam, rIter,
                    "IsValidKeepPreSelfPoseNextCameraByParam");
    tryGetByamlBool(&isInvalidKeepPreSelfPoseNextCameraOverWriteProgram, rIter,
                    "IsInvalidKeepPreSelfPoseNextCameraOverWriteProgram");
    tryGetByamlBool(&isInvalidKeepDistanceNextCamera, rIter, "IsInvalidKeepDistanceNextCamera");
}

/**
 * Returns whether the next camera keeps the pose of this camera.
 * @return Whether the pose is kept.
 */
bool CameraPoserFlag::isValidKeepPreSelfPoseNextCamera() const {
    return isOverWriteProgram ? !isInvalidKeepPreSelfPoseNextCameraOverWriteProgram :
                                isValidKeepPreSelfPoseNextCameraByParam;
}

}  // namespace al
