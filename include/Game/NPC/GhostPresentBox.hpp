#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GhostPresentBox : public al::LiveActor {
public:
    GhostPresentBox(const char*, int);
    void start();
private:
    unsigned char _144[0x160 - 0x144];
};
