#pragma once

#include <basis/seadTypes.h>

/// Actions that run on top of the main one, like throwing (implemented by PlayerSubAction).
class IUsePlayerSubAction {
public:
    virtual void validateAll() = 0;
    virtual void invalidateAll() = 0;
    virtual void validate(u32) = 0;
    virtual void invalidate(u32) = 0;
    virtual bool isValid(u32) const = 0;
    virtual void setThrowAnimCancel(bool) = 0;
    virtual void setMainAnimAfterThrow(const char*) = 0;
    virtual void setMainAnimAfterTailAttack(const char*) = 0;
    virtual void setIgnoreFloorCondition(bool) = 0;
    virtual bool isRunning() const = 0;
    virtual void forceEnd() = 0;
};
