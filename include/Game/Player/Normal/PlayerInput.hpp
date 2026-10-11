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

    u32 getInvalidationFrames() const { return mInvalidationFrames; }

private:
    // Input state preceding the frame-based invalidation counter remains unrecovered.
    u8 _008[0x6c];
    u32 mInvalidationFrames;
};
