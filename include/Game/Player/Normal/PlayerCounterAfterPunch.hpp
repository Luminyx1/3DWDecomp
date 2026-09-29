#pragma once

#include <basis/seadTypes.h>

class PlayerTrigger;

/// Counts the frames since the player's last punch (cat or Bowser's Fury giga punch).
class PlayerCounterAfterPunch {
public:
    PlayerCounterAfterPunch();

    void update(const PlayerTrigger*);

    u32 getCounter() const { return mCounter; }

private:
    u32 mCounter;  // 0x0
};
