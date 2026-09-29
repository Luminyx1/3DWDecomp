#include "Library/Collision/CollisionPartsConnector.hpp"

#include "Project/Collision/CollisionParts.hpp"

namespace al {

/**
 * @brief Connects to a parent matrix and remembers the collision parts that own it.
 * @param pParentMtx The matrix to follow.
 * @param rMtx The offset matrix relative to the parent.
 * @param pCollisionParts The collision parts the parent matrix belongs to.
 */
void CollisionPartsConnector::init(const sead::Matrix34f* pParentMtx, const sead::Matrix34f& rMtx,
                                   const CollisionParts* pCollisionParts) {
    MtxConnector::init(pParentMtx, rMtx);
    mCollisionParts = pCollisionParts;
}

/**
 * @brief Checks whether the connector follows a matrix whose collision is still valid.
 * @return True if connected and the collision parts are valid.
 */
bool CollisionPartsConnector::isConnecting() const {
    if (mCollisionParts && !(mCollisionParts->_160 && mCollisionParts->_161)) {
        return false;
    }

    return MtxConnector::isConnecting();
}

/**
 * @brief Disconnects from the parent matrix and forgets the collision parts.
 */
void CollisionPartsConnector::clear() {
    MtxConnector::clear();
    mCollisionParts = nullptr;
}

}  // namespace al
