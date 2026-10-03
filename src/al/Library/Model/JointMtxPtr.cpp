#include "Library/Model/JointMtxPtr.hpp"

namespace al {

/**
 * Constructs a null joint matrix pointer.
 */
JointMtxPtr::JointMtxPtr() : mMtx34(nullptr), mIsMatrix43(false) {}

/**
 * Clears the pointer.
 */
void JointMtxPtr::setNull() {
    mMtx34 = nullptr;
    mIsMatrix43 = false;
}

/**
 * Points to a row-major matrix.
 * @param pMtx Matrix.
 */
void JointMtxPtr::set(const sead::Matrix34f* pMtx) {
    mMtx34 = pMtx;
    mIsMatrix43 = false;
}

/**
 * Points to a column-major matrix.
 * @param pMtx Matrix.
 */
void JointMtxPtr::set(const Matrix43f* pMtx) {
    mMtx43 = pMtx;
    mIsMatrix43 = true;
}

/**
 * Gets the translation of the matrix.
 * @param pOut Output translation.
 */
void JointMtxPtr::getTranslation(sead::Vector3f* pOut) const {
    if (mIsMatrix43) {
        pOut->set(mMtx43->m[3][0], mMtx43->m[3][1], mMtx43->m[3][2]);
        return;
    }

    const sead::Matrix34f* mtx = mMtx34;
    pOut->x = mtx->m[0][3];
    pOut->y = mtx->m[1][3];
    pOut->z = mtx->m[2][3];
}

/**
 * Calculates the scale of the matrix.
 * @param pOut Output scale.
 */
void JointMtxPtr::calcMtxScale(sead::Vector3f* pOut) const {
    if (mIsMatrix43) {
        al::calcMtxScale(pOut, *mMtx43);
        return;
    }

    al::calcMtxScale(pOut, *mMtx34);
}

/**
 * Copies the matrix into a row-major matrix.
 * @param pOut Output matrix.
 */
void JointMtxPtr::copyTo(sead::Matrix34f* pOut) const {
    if (mIsMatrix43) {
        const f32* mtx = &mMtx43->m[0][0];
        pOut->m[0][0] = mtx[0];
        pOut->m[0][1] = mtx[4];
        pOut->m[0][2] = mtx[8];
        pOut->m[0][3] = mtx[12];
        pOut->m[1][0] = mtx[1];
        pOut->m[1][1] = mtx[5];
        pOut->m[1][2] = mtx[9];
        pOut->m[1][3] = mtx[13];
        pOut->m[2][0] = mtx[2];
        pOut->m[2][1] = mtx[6];
        pOut->m[2][2] = mtx[10];
        pOut->m[2][3] = mtx[14];
        return;
    }

    *pOut = *mMtx34;
}

}  // namespace al
