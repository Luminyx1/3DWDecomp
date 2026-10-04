#pragma once

#include "Library/LiveActor/LiveActor.hpp"

/// Bubble that carries a player who fell behind back to the others.
class TractorBubble : public al::LiveActor {
public:
    bool isPlayerInBubble();
};
