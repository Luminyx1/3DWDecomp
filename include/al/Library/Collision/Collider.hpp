#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Collision/CollisionPolygonUtil.hpp"
#include "Project/Collision/HitDb.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace al {
class CollisionDirector;
class CollisionPartsFilterBase;
class SphereInterpolator;
class TriangleFilterBase;

class Collider : public IUseCollision {
public:
    Collider(CollisionDirector* pDirector, const sead::Matrix34f* pBaseMtx,
             const sead::Vector3f* pTrans, const sead::Vector3f* pGravity, f32 radius,
             f32 offsetY, u32 planeNum);

    void clear();
    void setTriangleFilter(const TriangleFilterBase* pFilter);
    void setCollisionPartsFilter(const CollisionPartsFilterBase* pFilter);
    void updateRecentOnGroundInfo();
    void clearStoredPlaneNum();
    void clearContactPlane();
    void onInvalidate();
    void calcCheckPos(sead::Vector3f* pPos) const;
    const sead::Vector3f& getRecentOnGroundNormal(u32 index) const;
    HitInfo* getPlane(s32 index) const;
    bool calcMovePowerByContact(sead::Vector3f* pMovePower, const sead::Vector3f& rCheckPos);
    u32 storeCurrentHitInfo(SphereHitInfo* pHitInfos, u32 maxNum);
    void obtainMomentFixReaction(SphereHitInfo* pHitInfos, sead::Vector3f* pFixReaction,
                                 sead::Vector3f* pFixNormalReaction, bool isFirst, u32 startIndex);
    void storeContactPlane(SphereHitInfo* pHitInfos);
    sead::Vector3f collide(const sead::Vector3f& rMove);
    bool preCollide(SphereInterpolator* pInterp, sead::Vector3f* pPos, f32* pRadius,
                    const sead::Vector3f& rMove, SphereHitInfo* pHitInfos, u32 maxNum);
    bool findCollidePos(s32* pHitNum, SphereInterpolator* pInterp, SphereHitInfo* pHitInfos,
                        u32 maxNum);

    CollisionDirector* getCollisionDirector() const override;

    f32 getRadius() const { return mRadius; }

    f32 getOffsetY() const { return mOffsetY; }

    void setRadius(f32 radius) { mRadius = radius; }

    void setOffsetY(f32 offsetY) { mOffsetY = offsetY; }

    void setReactMovePower(bool isEnabled) { mIsReactMovePower = isEnabled; }

    bool isCollidedFloor() const { return _110 >= 0.0f; }

    bool isCollidedWall() const { return _1b8 >= 0.0f; }

    bool isCollidedCeiling() const { return _260 >= 0.0f; }

    CollisionDirector* mCollisionDirector = nullptr;
    const TriangleFilterBase* mTriFilterBase = nullptr;
    const CollisionPartsFilterBase* mColFilterBase = nullptr;
    const sead::Matrix34f* mBaseMtx = nullptr;
    const sead::Vector3f* mTrans = nullptr;
    const sead::Vector3f* mGravity = nullptr;
    f32 mRadius = 0.0f;
    f32 mOffsetY = 0.0f;
    const sead::Vector3f* mCheckOffset = nullptr;
    u32 mPlaneNum = 0;
    u32 mStoredPlaneNum = 0;
    HitInfo* mPlanes = nullptr;
    sead::Vector3f mFixReaction = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mMovePower = {0.0f, 0.0f, 0.0f};
    HitInfo mFloor;
    f32 _110 = 0.0f;
    HitInfo mWall;
    f32 _1b8 = 0.0f;
    HitInfo mCeiling;
    f32 _260 = 0.0f;
    u32 _264 = 0;
    sead::Vector3f mRecentOnGroundNormal = {0.0f, 1.0f, 0.0f};
    bool mIsReactMovePower : 1;
    bool mIsCheckMovingReaction : 1;
    bool mIsRotateCheckOffset : 1;
    bool mIsCollidedFloorFace : 1;
    bool mIsCollidedWallFace : 1;
    bool mIsCollidedCeilingFace : 1;
    sead::Vector3f mCurrentTrans;
    f32 mCurrentRadius;
};

static_assert(sizeof(Collider) == 0x288);
}  // namespace al
