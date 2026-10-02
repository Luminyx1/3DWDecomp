#include "Library/Play/Camera/CameraPoserProgramable_RS.hpp"

namespace al {

/**
 * Constructs a poser whose pose is read from externally owned vectors.
 * @param pPos Camera position source, may be null.
 * @param pAt Look-at position source, may be null.
 * @param pUp Up direction source, may be null.
 */
CameraPoserProgramable_RS::CameraPoserProgramable_RS(const sead::Vector3f* pPos,
                                                     const sead::Vector3f* pAt,
                                                     const sead::Vector3f* pUp)
    : CameraPoser_RS("プログラマブル"), mPosPtr(pPos), mAtPtr(pAt), mUpPtr(pUp) {}

/**
 * Constructs a poser without pose sources; the pose is set through setPose().
 */
CameraPoserProgramable_RS::CameraPoserProgramable_RS()
    : CameraPoser_RS("プログラマブル"), mPosPtr(nullptr), mAtPtr(nullptr), mUpPtr(nullptr) {}

/**
 * Copies the pose from every source vector that is set.
 */
void CameraPoserProgramable_RS::update() {
    if (mPosPtr != nullptr) {
        mEye.set(*mPosPtr);
    }

    if (mAtPtr != nullptr) {
        mAt.set(*mAtPtr);
    }

    if (mUpPtr != nullptr) {
        mUp.set(*mUpPtr);
    }
}

/**
 * Sets the camera pose directly.
 * @param rPos Camera position.
 * @param rAt Look-at position.
 * @param rUp Up direction.
 */
void CameraPoserProgramable_RS::setPose(const sead::Vector3f& rPos, const sead::Vector3f& rAt,
                                        const sead::Vector3f& rUp) {
    mEye.set(rPos);
    mAt.set(rAt);
    mUp.set(rUp);
}

}  // namespace al
