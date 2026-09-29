#pragma once

/// A bool-returning callback (implemented by PlayerBoolDelegateConst).
class IUsePlayerBoolDelegate {
public:
    virtual bool invoke() = 0;
};
