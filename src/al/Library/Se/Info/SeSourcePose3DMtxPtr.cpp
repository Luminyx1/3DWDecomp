#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a pose that follows a matrix.
 * @param pMtx Followed matrix.
 */
SeSourcePose3DMtxPtr::SeSourcePose3DMtxPtr(const sead::Matrix34f* pMtx)
    : SeSourcePose3DMtxBase("マトリクス"), mMtxPtr(pMtx) {}

/**
 * Updates the position from the followed matrix.
 */
void SeSourcePose3DMtxPtr::update() {
    mMtxPtr->getTranslation(mPos);
}
}  // namespace al
