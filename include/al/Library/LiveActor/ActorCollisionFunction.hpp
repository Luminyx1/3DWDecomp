#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorCollisionController;
class Collider;
class CollisionParts;
class CollisionPartsFilterBase;
class HitSensor;
class LiveActor;
class TriangleFilterBase;

bool isExistActorCollider(const LiveActor* pActor);
Collider* getActorCollider(const LiveActor* pActor);
const HitSensor* getActorCollisionPartsSensor(const LiveActor* pActor);
bool isExistCollisionParts(const LiveActor* pActor);
void validateCollisionParts(LiveActor* pActor);
void invalidateCollisionParts(LiveActor* pActor);
void validateCollisionPartsBySystem(LiveActor* pActor);
void invalidateCollisionPartsBySystem(LiveActor* pActor);
bool isValidCollisionParts(const LiveActor* pActor);
void setCollisionPartsSpecialPurposeName(LiveActor* pActor, const char* pName);
void resetAllCollisionMtx(LiveActor* pActor);
void syncCollisionMtx(LiveActor* pActor, const sead::Matrix34f* pMtx);
void syncCollisionMtx(LiveActor* pActor, CollisionParts* pCollisionParts, const sead::Matrix34f* pMtx);
void setSyncCollisionMtxPtr(LiveActor* pActor, const sead::Matrix34f* pMtx);
bool isOnGroundFace(const LiveActor* pActor);
bool isCollidedGroundEdgeOrCorner(const LiveActor* pActor);
bool isOnGroundNoVelocity(const LiveActor* pActor, u32 coyoteTime);
const sead::Vector3f& getOnGroundNormal(const LiveActor* pActor, u32 offset);
void setColliderRadius(LiveActor* pActor, f32 radius);
void setColliderOffsetY(LiveActor* pActor, f32 offsety);
f32 getColliderRadius(const LiveActor* pActor);
f32 getColliderOffsetY(const LiveActor* pActor);
void setColliderReactMovePower(LiveActor* pActor, bool isEnabled);
void calcCollidedNormalSum(const LiveActor* pActor, sead::Vector3f* pOutNormal);
void setColliderFilterTriangle(LiveActor* pActor, const TriangleFilterBase* pTriangleFilter);
void setColliderFilterCollisionParts(LiveActor* pActor, const CollisionPartsFilterBase* pCollisionPartsFilter);
void createAndSetColliderFilterExistActor(LiveActor* pActor);
void createAndSetColliderFilterExistActor(LiveActor* pActor, LiveActor* pFilterActor);
void createAndSetColliderSpecialPurpose(LiveActor* pActor, const char* pName);
void createAndSetColliderSpecialPurposeForCollisionActor(LiveActor* pActor, const char* pName);
ActorCollisionController* createActorCollisionController(LiveActor* pActor);
void setColliderRadius(ActorCollisionController* pCollisionController, f32 radius);
void setColliderOffsetY(ActorCollisionController* pCollisionController, f32 offsetY);
void resetActorCollisionController(ActorCollisionController* pCollisionController, s32 delay);
void updateActorCollisionController(ActorCollisionController* pCollisionController);
const char* getCollidedFloorMaterialCodeName(const LiveActor* pActor);
const char* getCollidedWallMaterialCodeName(const LiveActor* pActor);
const char* getCollidedCeilingMaterialCodeName(const LiveActor* pActor);
bool isCollided(const LiveActor* pActor);
bool isCollidedGround(const LiveActor* pActor);
bool isCollidedWall(const LiveActor* pActor);
bool isCollidedCeiling(const LiveActor* pActor);
bool isCollidedWallFace(const LiveActor* pActor);
bool isCollidedVelocity(const LiveActor* pActor);
bool isCollidedWallVelocity(const LiveActor* pActor);
bool isCollidedCeilingVelocity(const LiveActor* pActor);
const sead::Vector3f& getCollidedWallNormal(const LiveActor* pActor);
const sead::Vector3f& getCollidedCeilingNormal(const LiveActor* pActor);
const sead::Vector3f& getCollidedGroundNormal(const LiveActor* pActor);
const sead::Vector3f& getCollidedGroundPos(const LiveActor* pActor);
const sead::Vector3f& getCollidedWallPos(const LiveActor* pActor);
const sead::Vector3f& getCollidedCeilingPos(const LiveActor* pActor);
const CollisionParts* tryGetCollidedGroundCollisionParts(const LiveActor* pActor);
const CollisionParts* tryGetCollidedWallCollisionParts(const LiveActor* pActor);
const CollisionParts* tryGetCollidedCeilingCollisionParts(const LiveActor* pActor);
HitSensor* tryGetCollidedGroundSensor(const LiveActor* pActor);
HitSensor* tryGetCollidedWallSensor(const LiveActor* pActor);
HitSensor* tryGetCollidedCeilingSensor(const LiveActor* pActor);
void setForceCollisionScaleOne(const LiveActor* pActor);
void validateAllCollisionParts(LiveActor* pActor);
void invalidateAllCollisionParts(LiveActor* pActor);
void disableAllCollisionParts(LiveActor* pActor);
void enableAllCollisionParts(LiveActor* pActor);
bool isOnGround(const LiveActor* pActor, u32 checkFrame, f32 margin);
void calcColliderPos(const LiveActor* pActor, sead::Vector3f* pPos);
void calcColliderFloorRotatePower(LiveActor* pActor, sead::Quatf* pQuat);
void createAndSetColliderSpecialPurposeForCollisionMultiple(LiveActor* pActor, const char* pName, const char* pMultipleName);
const char* getCollidedFloorCodeName(const LiveActor* pActor);
const char* getCollidedFloorWallCodeName(const LiveActor* pActor);
const char* getCollidedWallCodeName(const LiveActor* pActor);
}  // namespace al
