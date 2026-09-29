#pragma once

#include <basis/seadTypes.h>

/// Blocks gliding for a while (implemented by PlayerGlideInhibitor).
class IUsePlayerGlideInhibitor {
public:
    virtual void requestInhibit(s32) = 0;
    virtual bool isInhibit() const = 0;
};
