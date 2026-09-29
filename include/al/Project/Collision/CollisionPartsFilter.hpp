#pragma once

#include <basis/seadTypes.h>

namespace al {
class CollisionParts;
class LiveActor;

class CollisionPartsFilterBase {
public:
    virtual bool isInvalidParts(const CollisionParts&) const = 0;
};

class CollisionPartsFilterActor : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterActor(const LiveActor* pActor) : mActor(pActor) {}

    bool isInvalidParts(const CollisionParts&) const override;

    const LiveActor* mActor;          // _8
    bool mIsInvalidSelf = true;       // _10
};
}  // namespace al
