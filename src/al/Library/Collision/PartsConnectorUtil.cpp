#include "Library/Collision/PartsConnectorUtil.hpp"

#include <gfx/seadColor.h>

#include "Library/Collision/CollisionPartsConnector.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsFilter.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"
#include "Project/Matrix/MatrixUtil.hpp"

namespace {
/**
 * @brief Reads the IsConnectToCollision placement flag.
 * @param rInfo The actor's init info.
 * @return True if the actor should follow collision.
 */
inline bool isConnectToCollision(const al::ActorInitInfo& rInfo) {
    bool isConnect = false;
    al::tryGetArg(&isConnect, rInfo, "IsConnectToCollision");
    return isConnect;
}
}  // namespace

namespace al {

/**
 * @brief Creates a connector using the actor's pose as base pose.
 * @param pActor The actor whose pose is used.
 * @return The new connector.
 */
MtxConnector* createMtxConnector(const LiveActor* pActor) {
    return new MtxConnector(getQuat(pActor), getTrans(pActor));
}

/**
 * @brief Creates a connector using a rotation and the actor's translation as base pose.
 * @param pActor The actor whose translation is used.
 * @param rQuat The base rotation.
 * @return The new connector.
 */
MtxConnector* createMtxConnector(const LiveActor* pActor, const sead::Quatf& rQuat) {
    return new MtxConnector(rQuat, getTrans(pActor));
}

/**
 * @brief Creates a connector if the placement asks to connect to collision.
 * @param pActor The actor whose pose is used.
 * @param rInfo The actor's init info.
 * @return The new connector, or null.
 */
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo) {
    return isConnectToCollision(rInfo) ? createMtxConnector(pActor) : nullptr;
}

/**
 * @brief Creates a connector with a base rotation if the placement asks to connect to collision.
 * @param pActor The actor whose translation is used.
 * @param rInfo The actor's init info.
 * @param rQuat The base rotation.
 * @return The new connector, or null.
 */
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo,
                                    const sead::Quatf& rQuat) {
    return isConnectToCollision(rInfo) ? createMtxConnector(pActor, rQuat) : nullptr;
}

/**
 * @brief Checks whether a connector is connected.
 * @param pConnector The connector.
 * @return True if connected.
 */
bool isMtxConnectorConnecting(const MtxConnector* pConnector) {
    return pConnector->isConnecting();
}

/**
 * @brief Connects to the collision parts hit by an arrow.
 * @param pConnector The connector.
 * @param pActor The actor whose own collision is ignored.
 * @param rPos The arrow start.
 * @param rDir The arrow direction and length.
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   const sead::Vector3f& rPos, const sead::Vector3f& rDir) {
    CollisionPartsFilterActor filter(pActor);
    CollisionParts* parts = alCollisionUtil::getStrikeArrowCollisionParts(pActor, nullptr, rPos, rDir,
                                                                          &filter, nullptr);
    if (!parts) {
        return;
    }

    attachMtxConnectorToCollisionParts(pConnector, parts);
}

/**
 * @brief Connects to the collision below or above the actor.
 * @param pConnector The connector.
 * @param pActor The actor.
 * @param isGround True to search downwards, false to search upwards.
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor, bool isGround) {
    attachMtxConnectorToCollision(pConnector, pActor, 50.0f, isGround ? -150.0f : 150.0f);
}

/**
 * @brief Connects to the collision along the actor's down direction.
 * @param pConnector The connector.
 * @param pActor The actor.
 * @param offsetUp How far above the actor the search starts.
 * @param distance How far the search goes down.
 */
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor, f32 offsetUp,
                                   f32 distance) {
    sead::Vector3f upDir;
    calcUpDir(&upDir, pActor);

    sead::Vector3f dir = upDir * -distance;
    sead::Vector3f pos = getTrans(pActor) + upDir * offsetUp;
    attachMtxConnectorToCollision(pConnector, pActor, pos, dir);
}

/**
 * @brief Connects to collision parts.
 * @param pConnector The connector.
 * @param pParts The collision parts to follow.
 */
void attachMtxConnectorToCollisionParts(MtxConnector* pConnector, const CollisionParts* pParts) {
    pConnector->init(&pParts->mBaseMtx, pParts->mBaseInvMtx);
}

/**
 * @brief Moves the actor to the connected base pose.
 * @param pActor The actor.
 * @param pConnector The connector.
 */
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector) {
    pConnector->multQT(getQuatPtr(pActor), getTransPtr(pActor));
}

/**
 * @brief Moves the actor to the connected pose of the given pose.
 * @param pActor The actor.
 * @param pConnector The connector.
 * @param rQuat The rotation to connect.
 * @param rTrans The translation to connect.
 */
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector, const sead::Quatf& rQuat,
                   const sead::Vector3f& rTrans) {
    pConnector->multQT(getQuatPtr(pActor), getTransPtr(pActor), rQuat, rTrans);
}

/**
 * @brief Moves the actor to the connected position of the given position.
 * @param pActor The actor.
 * @param pConnector The connector.
 * @param rTrans The translation to connect.
 */
void connectPoseTrans(LiveActor* pActor, const MtxConnector* pConnector, const sead::Vector3f& rTrans) {
    pConnector->multTrans(getTransPtr(pActor), rTrans);
}

/**
 * @brief Sets the actor pose to the connected matrix.
 * @param pActor The actor.
 * @param pConnector The connector.
 * @param rMtx The matrix to connect.
 */
void connectPoseMtx(LiveActor* pActor, const MtxConnector* pConnector, const sead::Matrix34f& rMtx) {
    sead::Matrix34f mtx;
    pConnector->multMtx(&mtx, rMtx);
    updatePoseMtx(pActor, &mtx);
}

/**
 * @brief Computes the connected pose of a pose.
 * @param pOutQuat Where the rotation is written.
 * @param pOutTrans Where the translation is written.
 * @param pConnector The connector.
 * @param rQuat The rotation to connect.
 * @param rTrans The translation to connect.
 */
void calcConnectQT(sead::Quatf* pOutQuat, sead::Vector3f* pOutTrans, const MtxConnector* pConnector,
                   const sead::Quatf& rQuat, const sead::Vector3f& rTrans) {
    pConnector->multQT(pOutQuat, pOutTrans, rQuat, rTrans);
}

/**
 * @brief Computes the connected matrix of a matrix.
 * @param pOut Where the matrix is written.
 * @param pConnector The connector.
 * @param rMtx The matrix to connect.
 */
void calcConnectMtx(sead::Matrix34f* pOut, const MtxConnector* pConnector, const sead::Matrix34f& rMtx) {
    pConnector->multMtx(pOut, rMtx);
}

/**
 * @brief Computes the connected matrix of a pose.
 * @param pOut Where the matrix is written.
 * @param pConnector The connector.
 * @param rQuat The rotation to connect.
 * @param rTrans The translation to connect.
 */
void calcConnectMtx(sead::Matrix34f* pOut, const MtxConnector* pConnector, const sead::Quatf& rQuat,
                    const sead::Vector3f& rTrans) {
    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);
    calcConnectMtx(pOut, pConnector, mtx);
}

/**
 * @brief Connects to the collision around the actor, keeping its rotation and translation.
 * @param pConnector The connector.
 * @param pActor The actor.
 * @param isUp True to search along the up direction, false for the opposite.
 * @param isUseHitPos True to use the hit position instead of the actor's translation.
 */
void attachMtxConnectorToCollisionRT(MtxConnector* pConnector, const LiveActor* pActor, bool isUp,
                                     bool isUseHitPos) {
    sead::Vector3f upDir;
    calcUpDir(&upDir, pActor);
    if (!isUp) {
        upDir = -upDir;
    }

    sead::Vector3f dir = upDir * 150.0f;
    sead::Vector3f pos = getTrans(pActor) - upDir * 50.0f;

    CollisionPartsFilterActor filter(pActor);
    sead::Vector3f hitPos;
    CollisionParts* parts =
        alCollisionUtil::getStrikeArrowCollisionParts(pActor, &hitPos, pos, dir, &filter, nullptr);
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
 * @brief Connects to the collision around the actor, keeping its quaternion and translation.
 * @param pConnector The connector.
 * @param pActor The actor.
 * @param isUp True to search along the up direction, false for the opposite.
 * @param isUseHitPos True to use the hit position instead of the actor's translation.
 */
void attachMtxConnectorToCollisionQT(MtxConnector* pConnector, const LiveActor* pActor, bool isUp,
                                     bool isUseHitPos) {
    sead::Vector3f upDir;
    calcUpDir(&upDir, pActor);
    if (!isUp) {
        upDir = -upDir;
    }

    sead::Vector3f dir = upDir * 150.0f;
    sead::Vector3f pos = getTrans(pActor) - upDir * 50.0f;

    CollisionPartsFilterActor filter(pActor);
    sead::Vector3f hitPos;
    CollisionParts* parts =
        alCollisionUtil::getStrikeArrowCollisionParts(pActor, &hitPos, pos, dir, &filter, nullptr);
    if (!parts) {
        return;
    }

    sead::Matrix34f mtx;
    mtx.makeQT(getQuat(pActor), isUseHitPos ? hitPos : getTrans(pActor));
    pConnector->init(&parts->mBaseMtx, parts->mBaseInvMtx * mtx);
}

/**
 * @brief Connects to a joint of an actor.
 * @param pConnector The connector.
 * @param pActor The actor that owns the joint.
 * @param pJointName The joint name.
 */
void attachMtxConnectorToJoint(MtxConnector* pConnector, const LiveActor* pActor, const char* pJointName) {
    attachMtxConnectorToMtxPtr(pConnector, getJointMtxPtr(pActor, pJointName));
}

/**
 * @brief Connects to the base matrix of an actor.
 * @param pConnector The connector.
 * @param pActor The actor to follow.
 * @param pMtx The offset matrix, or null for identity.
 */
void attachMtxConnectorToActor(MtxConnector* pConnector, const LiveActor* pActor,
                               const sead::Matrix34f* pMtx) {
    pConnector->init(pActor->getBaseMtx(), pMtx ? *pMtx : sead::Matrix34f::ident);
}

/**
 * @brief Connects to a matrix.
 * @param pConnector The connector.
 * @param pMtx The matrix to follow.
 */
void attachMtxConnectorToMtxPtr(MtxConnector* pConnector, const sead::Matrix34f* pMtx) {
    pConnector->init(pMtx, sead::Matrix34f::ident);
}

/**
 * @brief Connects to the collision parts of a triangle.
 * @param pConnector The connector.
 * @param rTriangle The triangle.
 * @param rMtx The pose to keep, in world space.
 */
void attachToHitTriangle(CollisionPartsConnector* pConnector, const Triangle& rTriangle,
                         const sead::Matrix34f& rMtx) {
    const CollisionParts* parts = rTriangle.mCollisionParts;
    pConnector->init(&parts->mBaseMtx, parts->mBaseInvMtx * rMtx, parts);
}

/**
 * @brief Connects to the collision parts of a hit.
 * @param pConnector The connector.
 * @param rHitInfo The hit.
 * @param rMtx The pose to keep, in world space.
 */
void attachToHitInfo(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo,
                     const sead::Matrix34f& rMtx) {
    attachToHitTriangle(pConnector, rHitInfo.mTriangle, rMtx);
}

/**
 * @brief Connects to a hit with -Z facing along the hit normal.
 * @param pConnector The connector.
 * @param rHitInfo The hit.
 */
void attachToHitInfoNrmToMinusZ(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo) {
    sead::Matrix34f mtx;
    makeMtxFrontNoSupportPos(&mtx, -*rHitInfo.mTriangle.getFaceNormal(), rHitInfo.mPos);
    attachToHitInfo(pConnector, rHitInfo, mtx);
}

/**
 * @brief Computes the connected pose of an offset transform.
 * @param pConnector The connector.
 * @param pOutTrans Where the translation is written, may be null.
 * @param pOutQuat Where the rotation is written, may be null.
 * @param pOutScale Where the scale is written, may be null.
 * @param rTrans The offset translation.
 * @param rRotate The offset rotation.
 */
void calcConnectInfo(const MtxConnector* pConnector, sead::Vector3f* pOutTrans, sead::Quatf* pOutQuat,
                     sead::Vector3f* pOutScale, const sead::Vector3f& rTrans,
                     const sead::Vector3f& rRotate) {
    pConnector->calcConnectInfo(pOutTrans, pOutQuat, pOutScale, rTrans, rRotate);
}

/**
 * @brief Moves the actor to the connected origin pose.
 * @param pActor The actor.
 * @param pConnector The connector.
 */
void connectPoseQTUsingConnectInfo(LiveActor* pActor, const MtxConnector* pConnector) {
    if (!pConnector->isConnecting()) {
        return;
    }

    sead::Vector3f trans;
    sead::Quatf quat;
    pConnector->calcConnectInfo(&trans, &quat, nullptr, sead::Vector3f::zero, sead::Vector3f::zero);
    setTrans(pActor, trans);
    setQuat(pActor, quat);
}

/**
 * @brief Gets the connector's base rotation.
 * @param pConnector The connector.
 * @return The base rotation.
 */
const sead::Quatf& getConnectBaseQuat(const MtxConnector* pConnector) {
    return pConnector->getBaseQuat();
}

/**
 * @brief Gets the connector's base translation.
 * @param pConnector The connector.
 * @return The base translation.
 */
const sead::Vector3f& getConnectBaseTrans(const MtxConnector* pConnector) {
    return pConnector->getBaseTrans();
}

}  // namespace al
