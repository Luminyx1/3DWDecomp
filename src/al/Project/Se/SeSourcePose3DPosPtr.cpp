#include "Project/Se/SeSourcePose3DPosPtr.hpp"

namespace al {
/**
 * @brief Constructs a sound source pose that follows a position owned elsewhere.
 * @param pPos Pointer to the position to follow.
 */
SeSourcePose3DPosPtr::SeSourcePose3DPosPtr(const sead::Vector3f* pPos) : SeSourcePose3D("3D位置"), mPos(pPos) {}
}  // namespace al
