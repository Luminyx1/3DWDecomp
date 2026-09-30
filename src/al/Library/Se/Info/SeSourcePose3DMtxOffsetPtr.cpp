#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a pose that follows a matrix with an offset.
 * @param pMtx Followed matrix.
 * @param pOffset Offset in the matrix space.
 */
SeSourcePose3DMtxOffsetPtr::SeSourcePose3DMtxOffsetPtr(const sead::Matrix34f* pMtx, const sead::Vector3f* pOffset)
    : SeSourcePose3DMtxBase("マトリクスとオフセット"), mMtxPtr(pMtx), mOffset(pOffset) {}

void SeSourcePose3DMtxOffsetPtr::update() {
    sead::Matrix34f offsetMtx;
    offsetMtx.makeT(*mOffset);
    mMtx.setMul(*mMtxPtr, offsetMtx);
    mMtx.getTranslation(mPos);
}
}  // namespace al
