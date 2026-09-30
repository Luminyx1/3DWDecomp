#include "Library/Collision/PartsConnectorUtil.hpp"

#include "Library/Collision/PartsConnector.hpp"
#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"

namespace al {
namespace {
inline bool isConnectToCollision(const ActorInitInfo& rInfo) {
    bool isConnect = false;
    tryGetArg(&isConnect, rInfo, "IsConnectToCollision");
    return isConnect;
}
}  // namespace

/**
 * Creates a connector with the current pose of an actor as base pose.
 * @param pActor actor
 * @return new connector
 */
MtxConnector* createMtxConnector(const LiveActor* pActor) {
    return new MtxConnector(getQuat(pActor), getTrans(pActor));
}

/**
 * Creates a connector with a rotation and the translation of an actor as base pose.
 * @param pActor actor
 * @param rQuat base rotation
 * @return new connector
 */
MtxConnector* createMtxConnector(const LiveActor* pActor, const sead::Quatf& rQuat) {
    return new MtxConnector(rQuat, getTrans(pActor));
}

/**
 * Creates a connector if the actor is placed with IsConnectToCollision.
 * @param pActor actor
 * @param rInfo actor init info
 * @return new connector, or null
 */
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (!isConnectToCollision(rInfo)) {
        return nullptr;
    }
    return createMtxConnector(pActor);
}

/**
 * Creates a connector with a base rotation if the actor is placed with IsConnectToCollision.
 * @param pActor actor
 * @param rInfo actor init info
 * @param rQuat base rotation
 * @return new connector, or null
 */
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo,
                                    const sead::Quatf& rQuat) {
    if (!isConnectToCollision(rInfo)) {
        return nullptr;
    }
    return createMtxConnector(pActor, rQuat);
}

/**
 * Checks whether a connector is connected.
 * @param pConnector connector
 * @return true if connected
 */
bool isMtxConnectorConnecting(const MtxConnector* pConnector) {
    return pConnector->isConnecting();
}

/**
 * Connects to the collision parts hit by an arrow, ignoring the actor itself.
 * @param pConnector connector
 * @param pActor actor
 * @param rPos arrow start
 * @param rDir arrow direction and length
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   const sead::Vector3f& rPos, const sead::Vector3f& rDir) {
    CollisionPartsFilterActor filter(pActor);
    CollisionParts* parts = alCollisionUtil::getStrikeArrowCollisionParts(pActor, nullptr, rPos,
                                                                          rDir, &filter, nullptr);
    if (!parts) {
        return;
    }
    attachMtxConnectorToCollisionParts(pConnector, parts);
}

/**
 * Connects to the collision below or above the actor.
 * @param pConnector connector
 * @param pActor actor
 * @param isAttachToGround true to check below the actor
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   bool isAttachToGround) {
    attachMtxConnectorToCollision(pConnector, pActor, 50.0f, isAttachToGround ? 150.0f : -150.0f);
}

/**
 * Connects to the collision along the up axis of the actor.
 * @param pConnector connector
 * @param pActor actor
 * @param checkOffsetUp offset of the arrow start along the up axis
 * @param checkDistance arrow length towards down
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   f32 checkOffsetUp, f32 checkDistance) {
    sead::Vector3f up;
    calcUpDir(&up, pActor);
    sead::Vector3f dir = up * -checkDistance;
    const sead::Vector3f& trans = getTrans(pActor);
    sead::Vector3f pos = up * checkOffsetUp + trans;
    attachMtxConnectorToCollision(pConnector, pActor, pos, dir);
}

/**
 * Connects to collision parts at their current pose.
 * @param pConnector connector
 * @param pParts collision parts
 */
void attachMtxConnectorToCollisionParts(MtxConnector* pConnector, const CollisionParts* pParts) {
    pConnector->init(&pParts->mBaseMtx, pParts->mBaseInvMtx);
}

/**
 * Moves an actor with the connection, starting from the base pose.
 * @param pActor actor
 * @param pConnector connector
 */
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector) {
    pConnector->multQT(getQuatPtr(pActor), getTransPtr(pActor));
}

/**
 * Moves an actor with the connection, starting from a pose.
 * @param pActor actor
 * @param pConnector connector
 * @param rQuat rotation to transform
 * @param rTrans translation to transform
 */
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector, const sead::Quatf& rQuat,
                   const sead::Vector3f& rTrans) {
    pConnector->multQT(getQuatPtr(pActor), getTransPtr(pActor), rQuat, rTrans);
}

/**
 * Moves the translation of an actor with the connection.
 * @param pActor actor
 * @param pConnector connector
 * @param rTrans translation to transform
 */
void connectPoseTrans(LiveActor* pActor, const MtxConnector* pConnector,
                      const sead::Vector3f& rTrans) {
    pConnector->multTrans(getTransPtr(pActor), rTrans);
}

/**
 * Moves an actor with the connection, starting from a matrix.
 * @param pActor actor
 * @param pConnector connector
 * @param rMtx matrix to transform
 */
void connectPoseMtx(LiveActor* pActor, const MtxConnector* pConnector,
                    const sead::Matrix34f& rMtx) {
    sead::Matrix34f mtx;
    pConnector->multMtx(&mtx, rMtx);
    updatePoseMtx(pActor, &mtx);
}

/**
 * Transforms a pose by a connection.
 * @param pQuat output rotation
 * @param pTrans output translation
 * @param pConnector connector
 * @param rQuat rotation to transform
 * @param rTrans translation to transform
 */
void calcConnectQT(sead::Quatf* pQuat, sead::Vector3f* pTrans, const MtxConnector* pConnector,
                   const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    pConnector->multQT(pQuat, pTrans, rQuat, rTrans);
}

/**
 * Transforms a matrix by a connection.
 * @param pMtx output matrix
 * @param pConnector connector
 * @param rMtx matrix to transform
 */
void calcConnectMtx(sead::Matrix34f* pMtx, const MtxConnector* pConnector,
                    const sead::Matrix34f& rMtx) {
    pConnector->multMtx(pMtx, rMtx);
}

/**
 * Transforms a pose by a connection into a matrix.
 * @param pMtx output matrix
 * @param pConnector connector
 * @param rQuat rotation to transform
 * @param rTrans translation to transform
 */
void calcConnectMtx(sead::Matrix34f* pMtx, const MtxConnector* pConnector,
                    const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);
    calcConnectMtx(pMtx, pConnector, mtx);
}

/**
 * Connects to the collision below or above an actor, keeping its rotation.
 * @param pConnector connector
 * @param pActor actor
 * @param isFacingUp true to check along the up axis, false along the down axis
 * @param isUseHitPos true to use the hit position instead of the actor translation
 */
void attachMtxConnectorToCollisionRT(MtxConnector* pConnector, const LiveActor* pActor,
                                     bool isFacingUp, bool isUseHitPos) {
    sead::Vector3f facing;
    calcUpDir(&facing, pActor);
    if (!isFacingUp) {
        facing = -facing;
    }
    sead::Vector3f dir = facing * 150.0f;
    sead::Vector3f pos = getTrans(pActor) - facing * 50.0f;

    CollisionPartsFilterActor filter(pActor);
    sead::Vector3f hitPos;
    CollisionParts* parts = alCollisionUtil::getStrikeArrowCollisionParts(pActor, &hitPos, pos,
                                                                          dir, &filter, nullptr);
    if (!parts) {
        return;
    }

    const sead::Vector3f& rotate = getRotate(pActor);
    sead::Matrix34f mtx;
    mtx.makeRT({sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                sead::Mathf::deg2rad(rotate.z)},
               isUseHitPos ? hitPos : getTrans(pActor));
    pConnector->init(&parts->mBaseMtx, parts->mBaseInvMtx * mtx);
}

/**
 * Connects to the collision below or above an actor, keeping its rotation.
 * @param pConnector connector
 * @param pActor actor
 * @param isFacingUp true to check along the up axis, false along the down axis
 * @param isUseHitPos true to use the hit position instead of the actor translation
 */
void attachMtxConnectorToCollisionQT(MtxConnector* pConnector, const LiveActor* pActor,
                                     bool isFacingUp, bool isUseHitPos) {
    sead::Vector3f facing;
    calcUpDir(&facing, pActor);
    if (!isFacingUp) {
        facing = -facing;
    }
    sead::Vector3f dir = facing * 150.0f;
    sead::Vector3f pos = getTrans(pActor) - facing * 50.0f;

    CollisionPartsFilterActor filter(pActor);
    sead::Vector3f hitPos;
    CollisionParts* parts = alCollisionUtil::getStrikeArrowCollisionParts(pActor, &hitPos, pos,
                                                                          dir, &filter, nullptr);
    if (!parts) {
        return;
    }

    sead::Matrix34f mtx;
    mtx.makeQT(getQuat(pActor), isUseHitPos ? hitPos : getTrans(pActor));
    pConnector->init(&parts->mBaseMtx, parts->mBaseInvMtx * mtx);
}

/**
 * Connects to a joint of an actor at the joint pose.
 * @param pConnector connector
 * @param pActor actor
 * @param pJointName joint name
 */
void attachMtxConnectorToJoint(MtxConnector* pConnector, const LiveActor* pActor,
                               const char* pJointName) {
    attachMtxConnectorToMtxPtr(pConnector, getJointMtxPtr(pActor, pJointName));
}

/**
 * Connects to the base matrix of an actor.
 * @param pConnector connector
 * @param pActor actor
 * @param pMtx matrix relative to the actor, null for identity
 */
void attachMtxConnectorToActor(MtxConnector* pConnector, const LiveActor* pActor,
                               const sead::Matrix34f* pMtx) {
    pConnector->init(pActor->getBaseMtx(), pMtx ? *pMtx : sead::Matrix34f::ident);
}

/**
 * Connects to a matrix with an identity relative matrix.
 * @param pConnector connector
 * @param pMtx parent matrix
 */
void attachMtxConnectorToMtxPtr(MtxConnector* pConnector, const sead::Matrix34f* pMtx) {
    pConnector->init(pMtx, sead::Matrix34f::ident);
}

/**
 * Connects to the collision parts of a triangle.
 * @param pConnector collision parts connector
 * @param rTriangle hit triangle
 * @param rMtx world matrix to keep relative to the collision parts
 */
void attachToHitTriangle(CollisionPartsConnector* pConnector, const Triangle& rTriangle,
                         const sead::Matrix34f& rMtx) {
    const CollisionParts* parts = rTriangle.mCollisionParts;
    pConnector->init(&parts->mBaseMtx, parts->mBaseInvMtx * rMtx, parts);
}

/**
 * Connects to the collision parts of a hit.
 * @param pConnector collision parts connector
 * @param rHitInfo hit info
 * @param rMtx world matrix to keep relative to the collision parts
 */
void attachToHitInfo(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo,
                     const sead::Matrix34f& rMtx) {
    attachToHitTriangle(pConnector, rHitInfo.mTriangle, rMtx);
}

/**
 * Connects to the collision parts of a hit with -Z facing along the hit normal.
 * @param pConnector collision parts connector
 * @param rHitInfo hit info
 */
void attachToHitInfoNrmToMinusZ(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo) {
    sead::Matrix34f mtx;
    makeMtxFrontNoSupportPos(&mtx, -*rHitInfo.mTriangle.getFaceNormal(), rHitInfo.mPos);
    attachToHitInfo(pConnector, rHitInfo, mtx);
}

/**
 * Calculates the connected pose of an offset.
 * @param pConnector connector
 * @param pTrans output translation, may be null
 * @param pQuat output rotation, may be null
 * @param pScale output scale, may be null
 * @param rOffsetTrans offset translation
 * @param rOffsetRotate offset rotation in radians
 */
void calcConnectInfo(const MtxConnector* pConnector, sead::Vector3f* pTrans, sead::Quatf* pQuat,
                     sead::Vector3f* pScale, const sead::Vector3f& rOffsetTrans,
                     const sead::Vector3f& rOffsetRotate) {
    pConnector->calcConnectInfo(pTrans, pQuat, pScale, rOffsetTrans, rOffsetRotate);
}

/**
 * Moves an actor to the connected pose.
 * @param pActor actor
 * @param pConnector connector
 */
void connectPoseQTUsingConnectInfo(LiveActor* pActor, const MtxConnector* pConnector) {
    if (!pConnector->isConnecting()) {
        return;
    }
    sead::Vector3f trans;
    sead::Quatf quat;
    pConnector->calcConnectInfo(&trans, &quat, nullptr, sead::Vector3f::zero,
                                sead::Vector3f::zero);
    setTrans(pActor, trans);
    setQuat(pActor, quat);
}

/**
 * Gets the base rotation of a connector.
 * @param pConnector connector
 * @return base rotation
 */
const sead::Quatf& getConnectBaseQuat(const MtxConnector* pConnector) {
    return pConnector->getBaseQuat();
}

/**
 * Gets the base translation of a connector.
 * @param pConnector connector
 * @return base translation
 */
const sead::Vector3f& getConnectBaseTrans(const MtxConnector* pConnector) {
    return pConnector->getBaseTrans();
}
}  // namespace al
