#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs the matrix pose base.
 * @param rName Pose name.
 */
SeSourcePose3DMtxBase::SeSourcePose3DMtxBase(const sead::SafeString& rName) : SeSourcePose3D(rName) {}
}  // namespace al
