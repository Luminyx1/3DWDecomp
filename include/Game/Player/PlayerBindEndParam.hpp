#pragma once

/// How a bind ended, as the binding object reported it.
class PlayerBindEndParam {
public:
    unsigned char _0[0x38];
    bool mIsInhibitWall;  // 0x38
};
