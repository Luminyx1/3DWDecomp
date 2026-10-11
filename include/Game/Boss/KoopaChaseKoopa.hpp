#pragma once
#include "Library/LiveActor/LiveActor.hpp"

class KoopaChaseKoopa : public al::LiveActor {
public:
    void doDamage();
    void doDamageSmall();
    bool isStateDamage() const;
    void setStateWarpProvocation();
    bool isStateWarpProvocation() const;

private:
    al::LiveActor* mHost;
};
