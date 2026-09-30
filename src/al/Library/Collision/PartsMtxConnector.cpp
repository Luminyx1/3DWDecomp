#include "Library/Connector/MtxConnector.hpp"

#include "Library/Math/MatrixUtil.hpp"

namespace al {
/**
 * Constructs an unconnected connector with a base pose.
 * @param rQuat base rotation
 * @param rTrans base translation
 */
MtxConnector::MtxConnector(const sead::Quatf& rQuat, const sead::Vector3f& rTrans)
    : mBaseQuat(rQuat), mBaseTrans(rTrans) {}

/**
 * Connects to a parent matrix with a relative matrix.
 * @param pParentMtx parent matrix, null to disconnect
 * @param rMtx matrix relative to the parent
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
 * Connects to a parent matrix at its current pose.
 * @param pParentMtx parent matrix, null to disconnect
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
 * Disconnects from the parent.
 */
void MtxConnector::clear() {
    mMtx = sead::Matrix34f::ident;
    mParentMtx = nullptr;
}

/**
 * Rotates a vector by the connection.
 * @param pOut output vector
 * @param rVec vector to rotate
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
 * Transforms a position by the connection.
 * @param pOut output position
 * @param rTrans position to transform
 */
void MtxConnector::multTrans(sead::Vector3f* pOut, const sead::Vector3f& rTrans) const {
    if (!isConnecting()) {
        pOut->set(rTrans);
        return;
    }

    pOut->setMul(*mParentMtx * mMtx, rTrans);
}

/**
 * Transforms a matrix by the connection.
 * @param pOut output matrix
 * @param rMtx matrix to transform
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
 * Transforms the base pose by the connection.
 * @param pQuat output rotation
 * @param pTrans output translation
 */
void MtxConnector::multQT(sead::Quatf* pQuat, sead::Vector3f* pTrans) const {
    multQT(pQuat, pTrans, mBaseQuat, mBaseTrans);
}

/**
 * Transforms a pose by the connection.
 * @param pOutQuat output rotation
 * @param pOutTrans output translation
 * @param rQuat rotation to transform
 * @param rTrans translation to transform
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
 * Gets the base rotation.
 * @return base rotation
 */
const sead::Quatf& MtxConnector::getBaseQuat() const {
    return mBaseQuat;
}

/**
 * Gets the base translation.
 * @return base translation
 */
const sead::Vector3f& MtxConnector::getBaseTrans() const {
    return mBaseTrans;
}

/**
 * Sets the base pose.
 * @param rQuat base rotation
 * @param rTrans base translation
 */
void MtxConnector::setBaseQuatTrans(const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    mBaseQuat = rQuat;
    mBaseTrans = rTrans;
}

/**
 * Checks whether the connector is connected to a parent.
 * @return true if connected
 */
bool MtxConnector::isConnecting() const {
    return mParentMtx != nullptr;
}

/**
 * Calculates the connected pose of an offset.
 * @param pTrans output translation, may be null
 * @param pQuat output rotation, may be null
 * @param pScale output scale, may be null
 * @param rOffsetTrans offset translation
 * @param rOffsetRotate offset rotation in radians
 */
void MtxConnector::calcConnectInfo(sead::Vector3f* pTrans, sead::Quatf* pQuat,
                                   sead::Vector3f* pScale, const sead::Vector3f& rOffsetTrans,
                                   const sead::Vector3f& rOffsetRotate) const {
    sead::Matrix34f mtx;
    calcMtxWithOffset(&mtx, rOffsetTrans, rOffsetRotate);

    if (pTrans) {
        mtx.getTranslation(*pTrans);
    }

    if (pQuat) {
        mtx.toQuat(*pQuat);
    }

    if (pScale) {
        calcMtxScale(pScale, mtx);
    }
}

/**
 * Calculates the connected matrix of an offset.
 * @param pOut output matrix
 * @param rOffsetTrans offset translation
 * @param rOffsetRotate offset rotation in radians
 */
void MtxConnector::calcMtxWithOffset(sead::Matrix34f* pOut, const sead::Vector3f& rOffsetTrans,
                                     const sead::Vector3f& rOffsetRotate) const {
    sead::Matrix34f transMtx;
    transMtx.makeT(rOffsetTrans);
    sead::Matrix34f rotateMtx;
    rotateMtx.makeR(rOffsetRotate);
    multMtx(pOut, transMtx * rotateMtx);
}

/**
 * Gets the translation of the parent matrix.
 * @param pTrans output translation
 * @return true if connected
 */
bool MtxConnector::tryGetParentTrans(sead::Vector3f* pTrans) const {
    if (!mParentMtx) {
        return false;
    }

    mParentMtx->getTranslation(*pTrans);
    return true;
}
}  // namespace al
