#pragma once
#include "Library/LiveActor/LiveActor.hpp"
// Partial layout used by the item carry state.
class DrcTouchPointer : public al::LiveActor {
public:
    const sead::Vector3f& getHitNormal() const { return mHitNormal; }
private:
    unsigned char _144[0x5c];
    sead::Vector3f mHitNormal;
};
