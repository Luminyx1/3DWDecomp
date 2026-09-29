#include "controller/seadControllerWrapper.h"

#include "prim/seadMemUtil.h"

namespace sead
{
const u8 ControllerWrapper::cPadConfigDefault[Controller::cPadIdx_Max] = {
    Controller::cPadIdx_A,
    Controller::cPadIdx_B,
    Controller::cPadIdx_C,
    Controller::cPadIdx_X,
    Controller::cPadIdx_Y,
    Controller::cPadIdx_Z,
    Controller::cPadIdx_2,
    Controller::cPadIdx_1,
    Controller::cPadIdx_Home,
    Controller::cPadIdx_Minus,
    Controller::cPadIdx_Plus,
    Controller::cPadIdx_Start,
    Controller::cPadIdx_Select,
    Controller::cPadIdx_L,
    Controller::cPadIdx_R,
    Controller::cPadIdx_Touch,
    Controller::cPadIdx_Up,
    Controller::cPadIdx_Down,
    Controller::cPadIdx_Left,
    Controller::cPadIdx_Right,
    Controller::cPadIdx_LeftStickUp,
    Controller::cPadIdx_LeftStickDown,
    Controller::cPadIdx_LeftStickLeft,
    Controller::cPadIdx_LeftStickRight,
    Controller::cPadIdx_RightStickUp,
    Controller::cPadIdx_RightStickDown,
    Controller::cPadIdx_RightStickLeft,
    Controller::cPadIdx_RightStickRight};

ControllerWrapper::ControllerWrapper()
{
    MemUtil::copy(mPadConfig, cPadConfigDefault, Controller::cPadIdx_Max);
}

void ControllerWrapper::calc(u32 prevHold, bool prevPointerOn)
{
    if (mIsEnable && mController && mController->isConnected())
    {
        mPadHold = BitFlag32(createPadMaskFromControllerPadMask_(mController->getHoldMask()));

        mLeftStick = mController->getLeftStick();
        mRightStick = mController->getRightStick();
        mLeftAnalogTrigger = mController->getLeftAnalogTrigger();
        mRightAnalogTrigger = mController->getRightAnalogTrigger();

        bool pointer_on = mController->isPointerOn();

        bool touchkey_hold = false;
        if (mTouchKeyBit >= 0)
        {
            touchkey_hold = mPadHold.isOnBit(mTouchKeyBit);
        }

        setPointerWithBound_(pointer_on, touchkey_hold, mController->getPointer());
        updateDerivativeParams_(createPadMaskFromControllerPadMask_(prevHold), prevPointerOn);
    }
    else
    {
        setIdle();
    }

    if (isIdle_())
    {
        mIdleFrame++;
    }
    else
    {
        mIdleFrame = 0;
    }
}

void ControllerWrapper::setPadConfig(s32 padbitMax, const u8* padConfig,
                                     bool enableStickcrossEmulation)
{
    if (padbitMax > 32)
    {
        return;
    }
    mPadBitMax = padbitMax;

    MemUtil::copy(mPadConfig, padConfig, padbitMax);

    mLeftStickCrossStartBit = -1;
    mRightStickCrossStartBit = -1;

    if (enableStickcrossEmulation)
    {
        for (s32 i = 0; i < padbitMax; i++)
        {
            if (padConfig[i] == Controller::cPadIdx_LeftStickUp)
            {
                mLeftStickCrossStartBit = i;
            }

            else if (padConfig[i] == Controller::cPadIdx_RightStickUp)
            {
                mRightStickCrossStartBit = i;
            }
        }
    }

    mTouchKeyBit = -1;

    for (s32 i = 0; i < padbitMax; i++)
    {
        if (padConfig[i] == Controller::cPadIdx_Touch)
        {
            mTouchKeyBit = i;
            break;
        }
    }
}

u32 ControllerWrapper::createPadMaskFromControllerPadMask_(u32 controllerMask) const
{
    BitFlag32 controller_pad_mask(controllerMask);
    BitFlag32 pad_mask;

    for (int i = 0; i < mPadBitMax; i++)
    {
        if (controller_pad_mask.isOnBit(mPadConfig[i]))
        {
            pad_mask.setBit(i);
        }
    }

    return pad_mask.getDirect();
}

}  // namespace sead
