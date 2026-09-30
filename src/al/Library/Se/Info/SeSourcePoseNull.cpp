#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a pose without position.
 */
SeSourcePoseNull::SeSourcePoseNull() : SeSourcePose("姿勢なし") {}

/**
 * Does nothing.
 */
void SeSourcePoseNull::update() {}
}  // namespace al
