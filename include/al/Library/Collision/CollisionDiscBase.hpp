#pragma once

#include <basis/seadTypes.h>
#include <container/seadRingBuffer.h>
#include <math/seadVector.h>

#include "Project/Collision/CollisionPartsTriangle.hpp"

namespace al {
class CollisionParts;
class KCollisionServer;
class KCPrismData;
class KCPrismHeader;
class LiveActor;

class CollisionDiscBase {
public:
    struct DiscHitInfo {
        Triangle mTriangle;
        sead::Vector3f mHitPos = sead::Vector3f::zero;
        f32 mDistance = 0.0f;
    };

    struct Result {
        KCPrismData* mPrismData = nullptr;
        const KCPrismHeader* mPrismHeader = nullptr;
        DiscHitInfo mHitInfo;
    };

    using ResultRingBuffer = sead::RingBuffer<Result>;

    virtual ResultRingBuffer* getResultRingBuffer() = 0;
    virtual const ResultRingBuffer* getResultRingBuffer() const = 0;

    bool check();
    void callbackFromParts(CollisionParts* pParts);
    u32 getHitNum() const;
    const DiscHitInfo* getHitInfo(u32 index) const;
    void callbackFromServer(KCPrismData* pData, const KCPrismHeader* pHeader);

protected:
    bool isResultFull() {
        const ResultRingBuffer* buffer = getResultRingBuffer();
        return buffer->size() >= buffer->capacity();
    }

    LiveActor* mActor;
    sead::Vector3f mPos;
    sead::Vector3f mDir;
    f32 mRadius;
    f32 mHeight;
    sead::Vector3f mLocalPos;
    sead::Vector3f mLocalDir;
    f32 mLocalRadius;
    KCollisionServer* mKCollisionServer;
    s32 mUpdatedHitNum;
};

static_assert(sizeof(CollisionDiscBase::Result) == 0x90);
}  // namespace al
