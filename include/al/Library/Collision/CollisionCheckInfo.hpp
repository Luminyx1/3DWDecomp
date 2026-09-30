#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

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
    f32 mRadius;
};

class ArrowCheckInfo : public CollisionCheckInfoBase {
public:
    const sead::Vector3f& getDir() const { return *mDir; }

    const sead::Vector3f* mDir;
};

class DiskCheckInfo : public CollisionCheckInfoBase {};

class HitResultBufferBase {
public:
    bool isFull() const { return mSize >= mCapacity; }

    s32 mSize;
    s32 mCapacity;
};

class SphereHitResultBuffer : public HitResultBufferBase {};

class ArrowHitResultBuffer : public HitResultBufferBase {};

class DiskHitResultBuffer : public HitResultBufferBase {};
}  // namespace al
