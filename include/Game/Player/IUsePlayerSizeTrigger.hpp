#pragma once

/// Triggers of the player's size changes (implemented by PlayerSizeTrigger).
class IUsePlayerSizeTrigger {
public:
    virtual bool isBigTrig() const = 0;
};
