#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/// Bubble that carries a player who fell behind back to the others.
class TractorBubble : public al::LiveActor {
public:
    TractorBubble(bool isSingleMode);

    bool isPlayerInBubble();
    bool isBindWait();
    bool isBubble();
    bool isEnableBubbleOutFrame(const al::LiveActor* pPlayer);
    bool isEnableBubbleInput(const al::LiveActor* pPlayer);
    void activatePlayerWithBubble(al::LiveActor* pPlayer);
    void startBubbleWithScreenOut(al::LiveActor* pPlayer);
    void startBubbleWithInput(al::LiveActor* pPlayer);

private:
    u8 _148[0x1f0 - 0x148];
};

static_assert(sizeof(TractorBubble) == 0x1f0);
