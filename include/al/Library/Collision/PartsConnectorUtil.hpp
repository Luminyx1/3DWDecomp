#pragma once

#include <basis/seadTypes.h>
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

MtxConnector* createMtxConnector(const LiveActor*);
MtxConnector* createMtxConnector(const LiveActor*, const sead::Quatf&);
MtxConnector* tryCreateMtxConnector(const LiveActor*, const ActorInitInfo&);
MtxConnector* tryCreateMtxConnector(const LiveActor*, const ActorInitInfo&, const sead::Quatf&);
bool isMtxConnectorConnecting(const MtxConnector*);
void attachMtxConnectorToCollision(MtxConnector*, const LiveActor*, const sead::Vector3f&,
                                   const sead::Vector3f&);
void attachMtxConnectorToCollision(MtxConnector*, const LiveActor*, bool);
void attachMtxConnectorToCollision(MtxConnector*, const LiveActor*, f32, f32);
void attachMtxConnectorToCollisionParts(MtxConnector*, const CollisionParts*);
void connectPoseQT(LiveActor*, const MtxConnector*);
void connectPoseQT(LiveActor*, const MtxConnector*, const sead::Quatf&, const sead::Vector3f&);
void connectPoseTrans(LiveActor*, const MtxConnector*, const sead::Vector3f&);
void connectPoseMtx(LiveActor*, const MtxConnector*, const sead::Matrix34f&);
void calcConnectQT(sead::Quatf*, sead::Vector3f*, const MtxConnector*, const sead::Quatf&,
                   const sead::Vector3f&);
void calcConnectMtx(sead::Matrix34f*, const MtxConnector*, const sead::Matrix34f&);
void calcConnectMtx(sead::Matrix34f*, const MtxConnector*, const sead::Quatf&, const sead::Vector3f&);
void attachMtxConnectorToCollisionRT(MtxConnector*, const LiveActor*, bool, bool);
void attachMtxConnectorToCollisionQT(MtxConnector*, const LiveActor*, bool, bool);
void attachMtxConnectorToJoint(MtxConnector*, const LiveActor*, const char*);
void attachMtxConnectorToActor(MtxConnector*, const LiveActor*, const sead::Matrix34f*);
void attachMtxConnectorToMtxPtr(MtxConnector*, const sead::Matrix34f*);
void attachToHitTriangle(CollisionPartsConnector*, const Triangle&, const sead::Matrix34f&);
void attachToHitInfo(CollisionPartsConnector*, const HitInfo&, const sead::Matrix34f&);
void attachToHitInfoNrmToMinusZ(CollisionPartsConnector*, const HitInfo&);
void calcConnectInfo(const MtxConnector*, sead::Vector3f*, sead::Quatf*, sead::Vector3f*,
                     const sead::Vector3f&, const sead::Vector3f&);
void connectPoseQTUsingConnectInfo(LiveActor*, const MtxConnector*);
const sead::Quatf& getConnectBaseQuat(const MtxConnector*);
const sead::Vector3f& getConnectBaseTrans(const MtxConnector*);
}  // namespace al
