#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class CollisionParts;
class HitSensor;
class KCPrismData;
class KCPrismHeader;

class Triangle {
public:
    Triangle();
    Triangle(const CollisionParts& rParts, KCPrismData* pData, const KCPrismHeader* pHeader);

    void fillData(const CollisionParts& rParts, KCPrismData* pData, const KCPrismHeader* pHeader);
    void fill(const sead::Vector3f& rPos0, const sead::Vector3f& rPos1,
              const sead::Vector3f& rPos2);
    bool isHostMoved() const;
    bool isValid() const;
    const sead::Vector3f* getNormal(s32 index) const;
    const sead::Vector3f* getFaceNormal() const;
    const sead::Vector3f* getEdgeNormal(s32 index) const;
    const sead::Vector3f* getPos(s32 index) const;
    sead::Vector3f* calcAndGetNormal(s32 index);
    sead::Vector3f* calcAndGetFaceNormal();
    sead::Vector3f* calcAndGetEdgeNormal(s32 index);
    sead::Vector3f* calcAndGetPos(s32 index);
    void calcCenterPos(sead::Vector3f* pCenter) const;
    void getLocalPos(sead::Vector3f* pPos, s32 index) const;
    void calcForceMovePower(sead::Vector3f* pPower, const sead::Vector3f& rPos) const;
    void calcForceRotatePower(sead::Quatf* pPower) const;
    bool getAttributes(ByamlIter* pIter) const;

    HitSensor* getSensor() const;
    const sead::Matrix34f* getBaseMtx() const;
    const sead::Matrix34f* getBaseInvMtx() const;
    const sead::Matrix34f* getPrevBaseMtx() const;

    const CollisionParts* mCollisionParts;
    KCPrismData* mPrismData;
    const KCPrismHeader* mPrismHeader;
    sead::Vector3f mNormals[4];
    sead::Vector3f mPos[3];
};
}  // namespace al

bool operator==(const al::Triangle& rLhs, const al::Triangle& rRhs);
bool operator!=(const al::Triangle& rLhs, const al::Triangle& rRhs);
