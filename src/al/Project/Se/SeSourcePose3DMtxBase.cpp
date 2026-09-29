#include "Project/Se/SeSourcePose.hpp"

namespace al {
/**
 * @brief Constructs the base of a sound source pose that has a full 3D matrix.
 * @param rName The name of the pose.
 */
SeSourcePose3DMtxBase::SeSourcePose3DMtxBase(const sead::SafeString& rName) : SeSourcePose3D(rName) {}
}  // namespace al
