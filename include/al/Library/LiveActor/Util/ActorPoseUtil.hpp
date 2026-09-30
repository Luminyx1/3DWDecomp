#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;

void initActorPoseT(LiveActor*, const sead::Vector3f&);
void setTrans(LiveActor*, const sead::Vector3f&);
void initActorPoseTR(LiveActor*, const sead::Vector3f&, const sead::Vector3f&);
void setRotate(LiveActor*, const sead::Vector3f&);
void makeMtxSRT(sead::Matrix34f*, const LiveActor*);
void makeMtxRT(sead::Matrix34f*, const LiveActor*);
void calcAnimFrontGravityPos(LiveActor*, const sead::Vector3f&);
const sead::Vector3f& getGravity(const LiveActor*);
const sead::Vector3f& getTrans(const LiveActor*);
const sead::Vector3f& getScale(const LiveActor*);
void copyPose(LiveActor*, const LiveActor*);
void updatePoseRotate(LiveActor*, const sead::Vector3f&);
void updatePoseQuat(LiveActor*, const sead::Quatf&);
void updatePoseMtx(LiveActor*, const sead::Matrix34f*);
void calcSideDir(sead::Vector3f*, const LiveActor*);
void calcUpDir(sead::Vector3f*, const LiveActor*);
void calcFrontDir(sead::Vector3f*, const LiveActor*);
void calcQuat(sead::Quatf*, const LiveActor*);
sead::Vector3f* getTransPtr(LiveActor*);
void setTrans(LiveActor*, f32, f32, f32);
void setTransX(LiveActor*, f32);
void setTransY(LiveActor*, f32);
void setTransZ(LiveActor*, f32);
const sead::Vector3f& getRotate(const LiveActor*);
sead::Vector3f* getRotatePtr(LiveActor*);
void setRotate(LiveActor*, f32, f32, f32);
void setRotateX(LiveActor*, f32);
void setRotateY(LiveActor*, f32);
void setRotateZ(LiveActor*, f32);
void rotateActor(LiveActor*, const sead::Vector3f&);
void rotateTranslateActor(LiveActor*, const sead::Vector3f&, const sead::Vector3f&);
sead::Vector3f* tryGetScalePtr(LiveActor*);
f32 getScaleX(const LiveActor*);
f32 getScaleY(const LiveActor*);
f32 getScaleZ(const LiveActor*);
void setScale(LiveActor*, const sead::Vector3f&);
void setScale(LiveActor*, f32, f32, f32);
void setScaleAll(LiveActor*, f32);
void setScaleX(LiveActor*, f32);
void setScaleY(LiveActor*, f32);
void setScaleZ(LiveActor*, f32);
const sead::Quatf& getQuat(const LiveActor*);
sead::Quatf* getQuatPtr(LiveActor*);
sead::Quatf* tryGetQuatPtr(LiveActor*);
void setQuat(LiveActor*, const sead::Quatf&);
void setGravity(const LiveActor*, const sead::Vector3f&);
const sead::Vector3f& getFront(const LiveActor*);
sead::Vector3f* getFrontPtr(LiveActor*);
void setFront(LiveActor*, const sead::Vector3f&);
void multVecPose(sead::Vector3f*, const LiveActor*, const sead::Vector3f&);
void multVecInvPose(sead::Vector3f*, const LiveActor*, const sead::Vector3f&);
void multVecInvQuat(sead::Vector3f*, const LiveActor*, const sead::Vector3f&);
void calcTransLocalOffset(sead::Vector3f*, const LiveActor*, const sead::Vector3f&);
f32 calcAngleOnPlaneDegreeToDirection(LiveActor*, const sead::Vector3f&);
f32 calcAngleOnPlaneDegreeToTarget(LiveActor*, const sead::Vector3f&);
f32 calcAngleOnPlaneDegreeToDirectionFixed(LiveActor*, const sead::Vector3f&, f32);
f32 calcAngleOnPlaneDegreeToTargetFixed(LiveActor*, const sead::Vector3f&, f32);
bool faceToDirection(LiveActor*, const sead::Vector3f&, f32, f32);
bool isActorObscured(const LiveActor*, f32, const sead::Vector3f*);
bool faceToTarget(LiveActor*, const sead::Vector3f&, f32, f32);
}  // namespace al

namespace alActorPoseFunction {
void calcBaseMtx(sead::Matrix34f*, const al::LiveActor*);
void updatePoseTRMSV(al::LiveActor*);
}  // namespace alActorPoseFunction
