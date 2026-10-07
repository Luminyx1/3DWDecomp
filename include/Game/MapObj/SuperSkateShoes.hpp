#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class SuperSkateRail;
class SuperSkateShoes : public al::LiveActor {
public:
    explicit SuperSkateShoes(const char*);
    void startGrind(SuperSkateRail*);
private:
    unsigned char _144[0x3c];
};
static_assert(sizeof(SuperSkateShoes) == 0x180);
