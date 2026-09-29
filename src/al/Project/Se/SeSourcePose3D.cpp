#include "Project/Se/SeSourcePose.hpp"

namespace al {
/**
 * @brief Constructs the base of a sound source pose that has a 3D position.
 * @param rName The name of the pose.
 */
SeSourcePose3D::SeSourcePose3D(const sead::SafeString& rName) : SeSourcePose(rName) {}
}  // namespace al
