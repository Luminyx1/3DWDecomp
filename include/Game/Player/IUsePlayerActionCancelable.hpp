#pragma once

/// An action that can be cancelled into another one.
class IUsePlayerActionCancelable {
public:
    virtual bool isPossibleToCancel() const = 0;
};
