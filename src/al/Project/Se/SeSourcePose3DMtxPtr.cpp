#include "Project/Se/SeSourcePose3DMtxPtr.hpp"

namespace al {
/**
 * @brief Constructs a sound source pose that follows a matrix owned elsewhere.
 * @param pMtx Pointer to the matrix to follow.
 */
SeSourcePose3DMtxPtr::SeSourcePose3DMtxPtr(const sead::Matrix34f* pMtx)
    : SeSourcePose3DMtxBase("マトリクス"), mMtx(pMtx) {}

/**
 * @brief Updates the cached position from the translation of the followed matrix.
 */
void SeSourcePose3DMtxPtr::update() {
    mPos.x = mMtx->m[0][3];
    mPos.y = mMtx->m[1][3];
    mPos.z = mMtx->m[2][3];
}
}  // namespace al
