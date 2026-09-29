#pragma once

#include <basis/seadTypes.h>

/// Damage invincibility after a hit, in statues, pipes, ... (implemented by PlayerDamageInvalidater).
class IUsePlayerDamageInvalidCheck {
public:
    virtual void invalidateForInfinite() = 0;
    virtual void validateForInfinite() = 0;
    virtual void invalidateForStatue() = 0;
    virtual void validateForStatue() = 0;
    virtual void invalidateForHelp() = 0;
    virtual void validateForHelp() = 0;
    virtual void invalidateDamage(u32) = 0;
    virtual bool isInvalid() const = 0;
    virtual void reset() = 0;
    virtual bool isFlashValid() const = 0;
    virtual void invalidateFlash() = 0;
    virtual void validateFlash() = 0;
    virtual bool isPipeInvalid() const = 0;
    virtual void invalidateForPipe() = 0;
    virtual void validateForPipe() = 0;
    virtual bool isInvalidFrame() const = 0;
    virtual bool isInfiniteValid() const = 0;
};
