#pragma once

/// A flag of the player's state (e.g. PlayerWallClimbInhibitFlag).
class IUsePlayerFlag {
public:
    virtual bool isOn() const = 0;
};
