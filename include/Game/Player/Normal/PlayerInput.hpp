#pragma once

#include <basis/seadTypes.h>

#include "Player/IUsePlayerInput.hpp"

class IUsePlayerKeyConfig;

/// The player's controller input, read from its pad port.
class PlayerInput : public IUsePlayerInput {
public:
    PlayerInput(const IUsePlayerKeyConfig* pKeyConfig);

    virtual void resetPrecedingJump();
    virtual void invalidateFrame(u32 frame);
    virtual void disableJumpButton();
    virtual void enableJumpButton();
};
