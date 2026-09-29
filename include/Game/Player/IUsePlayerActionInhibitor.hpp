#pragma once

/// Blocks an action until the player lands (e.g. PlayerPropellerInhibitor).
class IUsePlayerActionInhibitor {
public:
    virtual void inhibit() = 0;
    virtual bool isInhibit() const = 0;
};
