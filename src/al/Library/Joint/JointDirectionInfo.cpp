#include "Library/Joint/JointDirectionInfo.hpp"

namespace al {

/**
 * Constructs direction info aiming the local Y axis at world up with zero power.
 */
JointDirectionInfo::JointDirectionInfo() = default;

/**
 * Sets the joint-local direction that is turned toward the target.
 * @param rDir Local base direction.
 */
void JointDirectionInfo::setLocalBaseDir(const sead::Vector3f& rDir) {
    mLocalBaseDir.set(rDir);
}

/**
 * Sets the joint-local rotation axis.
 * @param rAxis Local rotation axis.
 */
void JointDirectionInfo::setLocalRotateAxis(const sead::Vector3f& rAxis) {
    mLocalRotateAxis.set(rAxis);
}

/**
 * Sets the world direction the joint should face.
 * @param rDir World target direction.
 */
void JointDirectionInfo::setWorldTargetDir(const sead::Vector3f& rDir) {
    mWorldTargetDir.set(rDir);
}

/**
 * Sets the blend rate, clamped to [0, 1].
 * @param rate New power rate.
 */
void JointDirectionInfo::setPowerRate(f32 rate) {
    if (rate < 0.0f) {
        rate = 0.0f;
    } else if (rate > 1.0f) {
        rate = 1.0f;
    }

    mPowerRate = rate;
}

/**
 * Sets the rotation limit.
 * @param degree Limit angle in degrees.
 */
void JointDirectionInfo::setLimitDegree(f32 degree) {
    mLimitDegree = degree;
}

/**
 * Increases the blend rate, clamped to [0, 1].
 * @param rate Amount to add.
 */
void JointDirectionInfo::addRate(f32 rate) {
    setPowerRate(mPowerRate + rate);
}

/**
 * Decreases the blend rate, clamped to [0, 1].
 * @param rate Amount to subtract.
 */
void JointDirectionInfo::subRate(f32 rate) {
    setPowerRate(mPowerRate - rate);
}

}  // namespace al
