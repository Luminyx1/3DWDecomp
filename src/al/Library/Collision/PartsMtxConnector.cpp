#include "Library/Connector/MtxConnector.hpp"

#include "Project/Matrix/MatrixUtil.hpp"

namespace al {

/**
 * @brief Constructs an unconnected connector with the given base pose.
 * @param rQuat The base rotation.
 * @param rTrans The base translation.
 */
MtxConnector::MtxConnector(const sead::Quatf& rQuat, const sead::Vector3f& rTrans)
    : mBaseQuat(rQuat), mBaseTrans(rTrans) {}

/**
 * @brief Connects to a parent matrix with an explicit offset matrix.
 * @param pParentMtx The matrix to follow. Clears the connector when null.
 * @param rMtx The offset matrix relative to the parent.
 */
void MtxConnector::init(const sead::Matrix34f* pParentMtx, const sead::Matrix34f& rMtx) {
    if (!pParentMtx) {
        clear();
        return;
    }

    mMtx = rMtx;
    mParentMtx = pParentMtx;
}

/**
 * @brief Connects to a parent matrix, keeping the current world pose.
 * @param pParentMtx The matrix to follow. Clears the connector when null.
 */
void MtxConnector::init(const sead::Matrix34f* pParentMtx) {
    if (!pParentMtx) {
        clear();
        return;
    }

    mMtx.setInverse(*pParentMtx);
    mParentMtx = pParentMtx;
}

/**
 * @brief Disconnects from the parent matrix.
 */
void MtxConnector::clear() {
    mMtx = sead::Matrix34f::ident;
    mParentMtx = nullptr;
}

/**
 * @brief Transforms a direction by the connection, ignoring translation.
 * @param pOut Where the transformed direction is written.
 * @param rVec The direction to transform.
 */
void MtxConnector::multVec(sead::Vector3f* pOut, const sead::Vector3f& rVec) const {
    if (!isConnecting()) {
        pOut->set(rVec);
        return;
    }

    sead::Matrix34f mtx = *mParentMtx * mMtx;
    mtx.setTranslation(sead::Vector3f::zero);
    pOut->setMul(mtx, rVec);
}

/**
 * @brief Transforms a position by the connection.
 * @param pOut Where the transformed position is written.
 * @param rVec The position to transform.
 */
void MtxConnector::multTrans(sead::Vector3f* pOut, const sead::Vector3f& rVec) const {
    if (!isConnecting()) {
        pOut->set(rVec);
        return;
    }

    pOut->setMul(*mParentMtx * mMtx, rVec);
}

/**
 * @brief Transforms a matrix by the connection.
 * @param pOut Where the transformed matrix is written.
 * @param rMtx The matrix to transform.
 */
void MtxConnector::multMtx(sead::Matrix34f* pOut, const sead::Matrix34f& rMtx) const {
    if (!isConnecting()) {
        *pOut = rMtx;
        return;
    }

    pOut->setMul(mMtx, rMtx);
    pOut->setMul(*mParentMtx, *pOut);
}

/**
 * @brief Transforms the base pose by the connection.
 * @param pOutQuat Where the resulting rotation is written.
 * @param pOutTrans Where the resulting translation is written.
 */
void MtxConnector::multQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans) const {
    multQT(pOutQuat, pOutTrans, mBaseQuat, mBaseTrans);
}

/**
 * @brief Transforms a pose by the connection.
 * @param pOutQuat Where the resulting rotation is written.
 * @param pOutTrans Where the resulting translation is written.
 * @param rQuat The rotation to transform.
 * @param rTrans The translation to transform.
 */
void MtxConnector::multQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans, const sead::Quatf& rQuat,
                          const sead::Vector3f& rTrans) const {
    if (!isConnecting()) {
        return;
    }

    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);
    multMtx(&mtx, mtx);
    mtx.toQuat(*pOutQuat);
    mtx.getTranslation(*pOutTrans);
}

/**
 * @brief Gets the base rotation.
 * @return The base rotation.
 */
const sead::Quatf& MtxConnector::getBaseQuat() const {
    return mBaseQuat;
}

/**
 * @brief Gets the base translation.
 * @return The base translation.
 */
const sead::Vector3f& MtxConnector::getBaseTrans() const {
    return mBaseTrans;
}

/**
 * @brief Sets the base pose.
 * @param rQuat The new base rotation.
 * @param rTrans The new base translation.
 */
void MtxConnector::setBaseQuatTrans(const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    mBaseQuat = rQuat;
    mBaseTrans = rTrans;
}

/**
 * @brief Checks whether a parent matrix is attached.
 * @return True if connected.
 */
bool MtxConnector::isConnecting() const {
    return mParentMtx != nullptr;
}

/**
 * @brief Computes the connected pose of an offset transform.
 * @param pOutTrans Where the translation is written, may be null.
 * @param pOutQuat Where the rotation is written, may be null.
 * @param pOutScale Where the scale is written, may be null.
 * @param rTrans The offset translation.
 * @param rRotate The offset rotation in radians.
 */
void MtxConnector::calcConnectInfo(sead::Vector3f* pOutTrans, sead::Quatf* pOutQuat,
                                   sead::Vector3f* pOutScale, const sead::Vector3f& rTrans,
                                   const sead::Vector3f& rRotate) const {
    sead::Matrix34f mtx;
    calcMtxWithOffset(&mtx, rTrans, rRotate);

    if (pOutTrans) {
        mtx.getTranslation(*pOutTrans);
    }

    if (pOutQuat) {
        mtx.toQuat(*pOutQuat);
    }

    if (pOutScale) {
        calcMtxScale(pOutScale, mtx);
    }
}

/**
 * @brief Computes the connected matrix of an offset transform.
 * @param pOut Where the matrix is written.
 * @param rTrans The offset translation.
 * @param rRotate The offset rotation in radians.
 */
void MtxConnector::calcMtxWithOffset(sead::Matrix34f* pOut, const sead::Vector3f& rTrans,
                                     const sead::Vector3f& rRotate) const {
    sead::Matrix34f offsetMtx;
    offsetMtx.makeT(rTrans);

    sead::Matrix34f rotateMtx;
    rotateMtx.makeR(rRotate);

    multMtx(pOut, offsetMtx * rotateMtx);
}

/**
 * @brief Gets the translation of the parent matrix.
 * @param pOut Where the translation is written.
 * @return True if connected.
 */
bool MtxConnector::tryGetParentTrans(sead::Vector3f* pOut) const {
    if (!mParentMtx) {
        return false;
    }

    mParentMtx->getTranslation(*pOut);
    return true;
}

}  // namespace al
