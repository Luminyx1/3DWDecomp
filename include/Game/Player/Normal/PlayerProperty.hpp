#pragma once

#include <math/seadVector.h>

/// The player's physical state shared by the actions.
struct PlayerProperty {
    sead::Vector3f mTrans;     // 0x0
    sead::Vector3f mFront;     // 0xc
    unsigned char _18[0xc];
    sead::Vector3f mVelocity;  // 0x24
    unsigned char _30[0x30];
    sead::Vector3f mGravity;   // 0x60
    sead::Vector3f mUpDir;     // 0x6c
};
