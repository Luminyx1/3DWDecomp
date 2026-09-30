#include "Library/Collision/PartsConnector.hpp"

#include "Project/Collision/CollisionParts.hpp"

namespace al {
/**
 * Constructs an unconnected collision parts connector.
 */
CollisionPartsConnector::CollisionPartsConnector() = default;

/**
 * Connects to a parent matrix owned by collision parts.
 * @param pParentMtx parent matrix
 * @param rMtx matrix relative to the parent
 * @param pParts collision parts owning the parent matrix
 */
void CollisionPartsConnector::init(const sead::Matrix34f* pParentMtx, const sead::Matrix34f& rMtx,
                                   const CollisionParts* pParts) {
    MtxConnector::init(pParentMtx, rMtx);
    mCollisionParts = pParts;
}

/**
 * Checks whether the connector is connected to valid collision parts.
 * @return true if connected
 */
bool CollisionPartsConnector::isConnecting() const {
    if (mCollisionParts && !(mCollisionParts->_160 && mCollisionParts->_161)) {
        return false;
    }

    return MtxConnector::isConnecting();
}

/**
 * Disconnects from the collision parts.
 */
void CollisionPartsConnector::clear() {
    MtxConnector::clear();
    mCollisionParts = nullptr;
}
}  // namespace al
