#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GigaBellPedestal;
class GigaBell : public al::LiveActor {
public:
    explicit GigaBell(const char*);
    void setBasePosition(sead::Vector3f);
    void setPedestal(GigaBellPedestal* pedestal) { mPedestal = pedestal; }
private:
    u8 mUnknown144[0x134];
    GigaBellPedestal* mPedestal;
    u8 mUnknown280[0x68];
};
static_assert(sizeof(GigaBell) == 0x2e8);
