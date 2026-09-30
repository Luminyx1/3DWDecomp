#pragma once

#include <math/seadMatrix.h>

#include "Library/Connector/MtxConnector.hpp"

namespace al {
class CollisionParts;

class CollisionPartsConnector : public MtxConnector {
public:
    CollisionPartsConnector();

    bool isConnecting() const override;
    void clear() override;

    void init(const sead::Matrix34f* pParentMtx, const sead::Matrix34f& rMtx,
              const CollisionParts* pParts);

    const CollisionParts* getCollisionParts() const { return mCollisionParts; }

private:
    const CollisionParts* mCollisionParts = nullptr;
};

static_assert(sizeof(CollisionPartsConnector) == 0x68);
}  // namespace al
