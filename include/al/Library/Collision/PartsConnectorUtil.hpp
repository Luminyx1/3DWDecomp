#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class CollisionParts;
class CollisionPartsConnector;
class HitInfo;
class LiveActor;
class MtxConnector;
class Triangle;

MtxConnector* createMtxConnector(const LiveActor* pActor);
MtxConnector* createMtxConnector(const LiveActor* pActor, const sead::Quatf& rQuat);
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo);
MtxConnector* tryCreateMtxConnector(const LiveActor* pActor, const ActorInitInfo& rInfo,
                                    const sead::Quatf& rQuat);
bool isMtxConnectorConnecting(const MtxConnector* pConnector);
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   const sead::Vector3f& rPos, const sead::Vector3f& rDir);
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   bool isAttachToGround);
void attachMtxConnectorToCollision(MtxConnector* pConnector, const LiveActor* pActor,
                                   f32 checkOffsetUp, f32 checkDistance);
void attachMtxConnectorToCollisionParts(MtxConnector* pConnector, const CollisionParts* pParts);
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector);
void connectPoseQT(LiveActor* pActor, const MtxConnector* pConnector, const sead::Quatf& rQuat,
                   const sead::Vector3f& rTrans);
void connectPoseTrans(LiveActor* pActor, const MtxConnector* pConnector,
                      const sead::Vector3f& rTrans);
void connectPoseMtx(LiveActor* pActor, const MtxConnector* pConnector,
                    const sead::Matrix34f& rMtx);
void calcConnectQT(sead::Quatf* pQuat, sead::Vector3f* pTrans, const MtxConnector* pConnector,
                   const sead::Quatf& rQuat, const sead::Vector3f& rTrans);
void calcConnectMtx(sead::Matrix34f* pMtx, const MtxConnector* pConnector,
                    const sead::Matrix34f& rMtx);
void calcConnectMtx(sead::Matrix34f* pMtx, const MtxConnector* pConnector,
                    const sead::Quatf& rQuat, const sead::Vector3f& rTrans);
void attachMtxConnectorToCollisionRT(MtxConnector* pConnector, const LiveActor* pActor,
                                     bool isFacingUp, bool isUseHitPos);
void attachMtxConnectorToCollisionQT(MtxConnector* pConnector, const LiveActor* pActor,
                                     bool isFacingUp, bool isUseHitPos);
void attachMtxConnectorToJoint(MtxConnector* pConnector, const LiveActor* pActor,
                               const char* pJointName);
void attachMtxConnectorToActor(MtxConnector* pConnector, const LiveActor* pActor,
                               const sead::Matrix34f* pMtx);
void attachMtxConnectorToMtxPtr(MtxConnector* pConnector, const sead::Matrix34f* pMtx);
void attachToHitTriangle(CollisionPartsConnector* pConnector, const Triangle& rTriangle,
                         const sead::Matrix34f& rMtx);
void attachToHitInfo(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo,
                     const sead::Matrix34f& rMtx);
void attachToHitInfoNrmToMinusZ(CollisionPartsConnector* pConnector, const HitInfo& rHitInfo);
void calcConnectInfo(const MtxConnector* pConnector, sead::Vector3f* pTrans, sead::Quatf* pQuat,
                     sead::Vector3f* pScale, const sead::Vector3f& rOffsetTrans,
                     const sead::Vector3f& rOffsetRotate);
void connectPoseQTUsingConnectInfo(LiveActor* pActor, const MtxConnector* pConnector);
const sead::Quatf& getConnectBaseQuat(const MtxConnector* pConnector);
const sead::Vector3f& getConnectBaseTrans(const MtxConnector* pConnector);
}  // namespace al
