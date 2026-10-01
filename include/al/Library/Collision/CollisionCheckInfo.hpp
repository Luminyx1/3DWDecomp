#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

#include "Project/Collision/HitDb.hpp"

namespace al {
class CollisionPartsFilterBase;
class TriangleFilterBase;

class CollisionCheckInfoBase {
public:
    const sead::Vector3f& getPos() const { return *mPos; }

    const CollisionPartsFilterBase* getPartsFilter() const { return mPartsFilter; }

    const TriangleFilterBase* getTriangleFilter() const { return mTriangleFilter; }

    const sead::Vector3f* mPos;
    const CollisionPartsFilterBase* mPartsFilter;
    const TriangleFilterBase* mTriangleFilter;
};

class SphereCheckInfo : public CollisionCheckInfoBase {
public:
    SphereCheckInfo() = default;

    SphereCheckInfo(const sead::Vector3f& rPos, f32 radius) {
        mPos = &rPos;
        mPartsFilter = nullptr;
        mTriangleFilter = nullptr;
        mRadius = radius;
    }

    f32 getRadius() const { return mRadius; }

    f32 mRadius;
};

class ArrowCheckInfo : public CollisionCheckInfoBase {
public:
    ArrowCheckInfo() = default;

    ArrowCheckInfo(const sead::Vector3f& rPos, const sead::Vector3f& rDir) {
        mPos = &rPos;
        mPartsFilter = nullptr;
        mTriangleFilter = nullptr;
        mDir = &rDir;
        sead::Vector3CalcCommon<f32>::add(mEndPos, rPos, rDir);
        mBoundBox.addPoint(rPos);
        mBoundBox.addPoint(mEndPos);
    }

    const sead::Vector3f& getDir() const { return *mDir; }

    const sead::Vector3f& getEndPos() const { return mEndPos; }

    const sead::BoundBox3f& getBoundBox() const { return mBoundBox; }

    const sead::Vector3f* mDir;
    sead::Vector3f mEndPos;
    sead::BoundBox3f mBoundBox;
};

class DiskCheckInfo : public CollisionCheckInfoBase {
public:
    DiskCheckInfo(const sead::Vector3f& rPos, f32 radius, f32 height, const sead::Vector3f& rDir);

    f32 getBoundingRadius() const { return mBoundingRadius; }

    f32 mRadius;
    f32 mHeight;
    const sead::Vector3f* mDir;
    f32 mBoundingRadius;
};

class SphereHitResultBuffer : public sead::ObjArray<SphereHitInfo> {};

class ArrowHitResultBuffer : public sead::ObjArray<ArrowHitInfo> {};

class DiskHitResultBuffer : public sead::ObjArray<DiskHitInfo> {};
}  // namespace al
