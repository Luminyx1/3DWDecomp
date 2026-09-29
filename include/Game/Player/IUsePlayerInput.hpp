#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

/// The player's controller input (implemented by PlayerInput).
class IUsePlayerInput {
public:
    virtual ~IUsePlayerInput() = default;
    virtual s32 getPort() const = 0;
    virtual bool isStickOn() const = 0;
    virtual const sead::Vector3f& getMoveVec() const = 0;
    virtual const sead::Vector3f& getMoveVecNoArrange() const = 0;
    virtual f32 getStickX() const = 0;
    virtual f32 getStickY() const = 0;
    virtual bool isRoundCW() const { return false; }
    virtual bool isRoundCCW() const { return false; }
    virtual bool isJumpTrigOn() const = 0;
    virtual bool isJumpButtonTrigOn() const = 0;
    virtual s32 getFrameFromLastJumpTrig() const = 0;
    virtual bool isJumpButtonOn() const = 0;
    virtual bool isDashTrigOn() const = 0;
    virtual bool isDashButtonOn() const = 0;
    virtual bool isDashButtonReleased() const = 0;
    virtual bool isSquatTrigOn() const = 0;
    virtual bool isSquatButtonOn() const = 0;
    virtual bool isHipDropTrigOn() const = 0;
    virtual bool isHipDropButtonOn() const = 0;
    virtual bool isFireBallTrigOn() const = 0;
    virtual bool isTailAttackTrigOn() const = 0;
    virtual bool isPrecedingSwimPaddleTrigOn() const = 0;
    virtual bool isSwimPaddleTrigOn() const = 0;
    virtual bool isSwimPaddleButtonOn() const = 0;
    virtual bool isStoneStatueTrigOn() const = 0;
    virtual bool isStoneStatueSustainButtonOn() const = 0;
    virtual bool isRollingTrigOn() const = 0;
    virtual bool isRollingButtonOn() const = 0;
    virtual bool isBubbleTrigOn() const = 0;
    virtual bool isClimbAttackTrigOn() const = 0;
    virtual bool isClimbAttackButtonOn() const = 0;
    virtual bool isHoldButtonOn() const = 0;
    virtual bool isHoldShakeOn() const = 0;
    virtual bool isHoldTrigOn() const = 0;
    virtual bool isReleaseTrigOn() const = 0;
    virtual bool isSpinAttackTrigOn() const = 0;
};
