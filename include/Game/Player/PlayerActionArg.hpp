#pragma once

class IUsePlayerInput;

/// The player systems every action gets handed.
struct PlayerActionArg {
    unsigned char _0[0x30];
    const IUsePlayerInput* mInput;  // 0x30
};
