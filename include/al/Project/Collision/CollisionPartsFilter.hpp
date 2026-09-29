#pragma once

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

private:
    const LiveActor* mActor;         // _8
    bool mIsCompareEqual = true;     // _10
};
}  // namespace al
