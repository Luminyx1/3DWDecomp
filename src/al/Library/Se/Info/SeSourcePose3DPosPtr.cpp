#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a pose that follows a position.
 * @param pPos Followed position.
 */
SeSourcePose3DPosPtr::SeSourcePose3DPosPtr(const sead::Vector3f* pPos) : SeSourcePose3D("3D位置"), mPosPtr(pPos) {}

/**
 * Does nothing.
 */
void SeSourcePose3DPosPtr::update() {}
}  // namespace al
