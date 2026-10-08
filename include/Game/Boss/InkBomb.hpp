#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/** @brief Ink bomb thrown by Fury Bowser that leaves ink puddles behind. */
class InkBomb : public al::LiveActor {
public:
    explicit InkBomb(const char* pName, const al::LiveActor* pHost = nullptr);

    /**
     * @brief Sets the actor that threw the bomb.
     * @param pHost Throwing actor.
     */
    void setHost(const al::LiveActor* pHost) { mHost = pHost; }

private:
    const al::LiveActor* mHost;  // 0x148
    u8 _150[0x1a8 - 0x150];
};
static_assert(sizeof(InkBomb) == 0x1a8);
