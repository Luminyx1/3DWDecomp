#pragma once

#include "Library/Connector/MtxConnector.hpp"

namespace al {
class LiveActor;
class CollisionParts;

class CollisionPartsConnector : public MtxConnector {
public:
    CollisionPartsConnector();

    bool isConnecting() const override;
    void clear() override;

    void init(const sead::Matrix34f*, const sead::Matrix34f&, const CollisionParts*);

private:
    const CollisionParts* mCollisionParts = nullptr;  // _60
};

static_assert(sizeof(CollisionPartsConnector) == 0x68);

void connectPoseQT(LiveActor*, const MtxConnector*);

void attachMtxConnectorToCollision(MtxConnector*, const LiveActor*, bool);
};  // namespace al
