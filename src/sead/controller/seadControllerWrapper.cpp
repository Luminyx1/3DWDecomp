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

/**
 * Constructs the wrapper with the identity pad mapping.
 */
ControllerWrapper::ControllerWrapper()
{
    MemUtil::copy(mPadConfig, cPadConfigDefault, Controller::cPadIdx_Max);
}

/**
 * Remaps the wrapped controller's input through the pad config, or goes idle if disabled.
 * @param prevHold hold mask of the wrapped controller in the previous frame
 * @param prevPointerOn whether the wrapped controller's pointer was on in the previous frame
 */
void ControllerWrapper::calc(u32 prevHold, bool prevPointerOn)
{
    if (mIsEnable && mController != nullptr && mController->isConnected())
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

/**
 * Sets the controller pad bit for each wrapper pad bit and locates the stick cross and touch bits.
 * @param padBitMax number of wrapper pad bits
 * @param pPadConfig controller pad bit for each wrapper pad bit
 * @param enableStickCrossEmulation whether to derive stick cross bits from the sticks
 */
void ControllerWrapper::setPadConfig(s32 padBitMax, const u8* pPadConfig,
                                     bool enableStickCrossEmulation)
{
    if (padBitMax > 32)
    {
        return;
    }

    mPadBitMax = padBitMax;

    MemUtil::copy(mPadConfig, pPadConfig, padBitMax);

    mLeftStickCrossStartBit = -1;
    mRightStickCrossStartBit = -1;

    if (enableStickCrossEmulation)
    {
        for (s32 i = 0; i < padBitMax; i++)
        {
            if (pPadConfig[i] == Controller::cPadIdx_LeftStickUp)
            {
                mLeftStickCrossStartBit = i;
            }
            else if (pPadConfig[i] == Controller::cPadIdx_RightStickUp)
            {
                mRightStickCrossStartBit = i;
            }
        }
    }

    mTouchKeyBit = -1;

    for (s32 i = 0; i < padBitMax; i++)
    {
        if (pPadConfig[i] == Controller::cPadIdx_Touch)
        {
            mTouchKeyBit = i;
            break;
        }
    }
}

/**
 * Converts a controller pad mask to a wrapper pad mask.
 * @param controllerMask controller pad mask
 * @return wrapper pad mask
 */
u32 ControllerWrapper::createPadMaskFromControllerPadMask_(u32 controllerMask) const
{
    BitFlag32 controller_pad_mask(controllerMask);
    BitFlag32 pad_mask;

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        if (controller_pad_mask.isOnBit(mPadConfig[i]))
        {
            pad_mask.setBit(i);
        }
    }

    return pad_mask.getDirect();
}

}  // namespace sead
