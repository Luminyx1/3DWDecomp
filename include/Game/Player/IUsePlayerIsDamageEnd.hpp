#pragma once

/// A damage (or bind) action that can report that the damage is over.
class IUsePlayerIsDamageEnd {
public:
    virtual bool isDamageEnd() const = 0;
};
