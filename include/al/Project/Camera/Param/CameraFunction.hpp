#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class LookAtCamera;
}

namespace al {
class ByamlIter;
class IUseAreaObj;
class IUseCollision;
class Resource;

namespace CameraFunction {
bool tryGetCameraIter(ByamlIter* pIter, const Resource* pResource, const char* pName);
void calcPosFromZoneToRoot(sead::Vector3f* pOut, const sead::Vector3f& rPos, const sead::Matrix34f& rZoneMtx);
void calcPosFromRootToZone(sead::Vector3f* pOut, const sead::Vector3f& rPos, const sead::Matrix34f& rZoneMtx);
f32 calcAngleHFromZoneToRoot(f32 angle, const sead::Matrix34f& rZoneMtx);
void calcCameraPosByRotateAngleHV(sead::Vector3f* pOut, const sead::LookAtCamera& rCamera, f32 angleH, f32 angleV);
s32 calcPolyNumOnArrow(const IUseCollision* pCollision, const sead::Vector3f& rStart, const sead::Vector3f& rArrow);
bool isInCollisionByBidirectionalCheck(const IUseCollision* pCollision, const sead::Vector3f& rStart,
                                       const sead::Vector3f& rEnd);
bool isInCollisionCameraPos(const IUseCollision* pCollision, const IUseAreaObj* pAreaObj,
                            const sead::LookAtCamera& rCamera, f32, f32);
bool checkValidCollisionBySphereHitInfo(const IUseCollision* pCollision, s32 index);
bool isInCameraNoRotateArea(const IUseAreaObj* pAreaObj, const sead::Vector3f& rPos);
}  // namespace CameraFunction
}  // namespace al
