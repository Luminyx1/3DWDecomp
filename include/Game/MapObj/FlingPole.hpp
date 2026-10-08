#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/**
 * @brief The flag pole raised next to a lighthouse once all of its Cat Shines are collected.
 * @note Only what reconstructed code needs is declared so far.
 */
class FlingPole : public al::LiveActor {
public:
    FlingPole(const char* pName, bool isLighthouseFlag);

    void showFlag(bool isShow);

private:
    u8 mUnreconstructed[0x248 - 0x144];
};

static_assert(sizeof(FlingPole) == 0x248);
