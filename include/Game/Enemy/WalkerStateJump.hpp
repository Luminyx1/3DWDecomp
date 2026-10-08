#pragma once

#include <basis/seadTypes.h>

/** @brief Tuning for WalkerStateJump (jump speed and actions). */
class WalkerStateJumpParam {
public:
    WalkerStateJumpParam();
    WalkerStateJumpParam(f32 jumpSpeed, bool isUseAction);
    WalkerStateJumpParam(f32 jumpSpeed, const char* pJumpAction, bool isUseAction);
    WalkerStateJumpParam(f32 jumpSpeed, const char* pJumpAction, const char* pFallAction,
                         const char* pLandAction, bool isUseAction);

private:
    alignas(8) u8 _0[0xB0];
};

static_assert(sizeof(WalkerStateJumpParam) == 0xB0);
