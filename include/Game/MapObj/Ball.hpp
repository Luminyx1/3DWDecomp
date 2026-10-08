#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/**
 * @brief Ball item that can be thrown at enemies.
 * @note Only the interface used by reconstructed code is declared so far.
 */
class Ball : public al::LiveActor {
public:
    explicit Ball(const char* pName);
    void appearPopUpFront();

private:
    u8 mUnreconstructed[0xa4];
};

static_assert(sizeof(Ball) == 0x1e8);
