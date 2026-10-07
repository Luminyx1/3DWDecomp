#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

class Gorobon : public al::LiveActor {
public:
    explicit Gorobon(const char* pName);

    void initAtBossStage();
    void forceBreak();

    bool mIsBossStage;  // 0x144
    bool mIsBossDemo;   // 0x145

private:
    u8 mUnreconstructed[0xe2];
};
static_assert(sizeof(Gorobon) == 0x228);
