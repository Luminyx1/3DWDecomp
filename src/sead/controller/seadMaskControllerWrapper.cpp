#include "controller/seadMaskControllerWrapper.h"

#include "prim/seadMemUtil.h"

namespace sead
{
const u32 MaskControllerWrapper::cPadConfigDefault[Controller::cPadIdx_Max] = {
    1 << Controller::cPadIdx_A,
    1 << Controller::cPadIdx_B,
    1 << Controller::cPadIdx_C,
    1 << Controller::cPadIdx_X,
    1 << Controller::cPadIdx_Y,
    1 << Controller::cPadIdx_Z,
    1 << Controller::cPadIdx_2,
    1 << Controller::cPadIdx_1,
    1 << Controller::cPadIdx_Home,
    1 << Controller::cPadIdx_Minus,
    1 << Controller::cPadIdx_Plus,
    1 << Controller::cPadIdx_Start,
    1 << Controller::cPadIdx_Select,
    1 << Controller::cPadIdx_L,
    1 << Controller::cPadIdx_R,
    1 << Controller::cPadIdx_Touch,
    1 << Controller::cPadIdx_Up,
    1 << Controller::cPadIdx_Down,
    1 << Controller::cPadIdx_Left,
    1 << Controller::cPadIdx_Right,
    1 << Controller::cPadIdx_LeftStickUp,
    1 << Controller::cPadIdx_LeftStickDown,
    1 << Controller::cPadIdx_LeftStickLeft,
    1 << Controller::cPadIdx_LeftStickRight,
    1 << Controller::cPadIdx_RightStickUp,
    1 << Controller::cPadIdx_RightStickDown,
    1 << Controller::cPadIdx_RightStickLeft,
    1 << Controller::cPadIdx_RightStickRight};

/**
 * Constructs the wrapper with a one-bit-per-pad mapping.
 */
MaskControllerWrapper::MaskControllerWrapper()
{
    MemUtil::copy(mPadConfig, cPadConfigDefault, sizeof(cPadConfigDefault));
}

/**
 * Remaps the wrapped controller's input through the pad masks, or goes idle if disabled.
 * @param prevHold hold mask of the wrapped controller in the previous frame
 * @param prevPointerOn whether the wrapped controller's pointer was on in the previous frame
 */
void MaskControllerWrapper::calc(u32 prevHold, bool prevPointerOn)
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

/**
 * Converts a controller pad mask to a wrapper pad mask; a wrapper bit is on if any of its controller bits are.
 * @param controllerMask controller pad mask
 * @return wrapper pad mask
 */
u32 MaskControllerWrapper::createPadMaskFromControllerPadMask_(u32 controllerMask) const
{
    u32 pad_mask = 0;

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        if (mPadConfig[i] & controllerMask)
        {
            pad_mask |= 1 << i;
        }
    }

    return pad_mask;
}

/**
 * Sets the controller pad mask for each wrapper pad bit and locates the stick cross and touch bits.
 * @param padBitMax number of wrapper pad bits
 * @param pPadConfig controller pad mask for each wrapper pad bit
 * @param enableStickCrossEmulation whether to derive stick cross bits from the sticks
 */
void MaskControllerWrapper::setPadConfig(s32 padBitMax, const u32* pPadConfig,
                                         bool enableStickCrossEmulation)
{
    if (padBitMax > cPadIdx_MaxBase)
    {
        return;
    }

    mPadBitMax = padBitMax;

    MemUtil::copy(mPadConfig, pPadConfig, padBitMax * sizeof(u32));

    mLeftStickCrossStartBit = -1;
    mRightStickCrossStartBit = -1;

    if (enableStickCrossEmulation)
    {
        for (s32 i = 0; i < padBitMax; i++)
        {
            if (pPadConfig[i] & (1 << Controller::cPadIdx_LeftStickUp))
            {
                mLeftStickCrossStartBit = i;
            }
            else if (pPadConfig[i] & (1 << Controller::cPadIdx_RightStickUp))
            {
                mRightStickCrossStartBit = i;
            }
        }
    }

    mTouchKeyBit = -1;

    for (s32 i = 0; i < padBitMax; i++)
    {
        if (pPadConfig[i] & (1 << Controller::cPadIdx_Touch))
        {
            mTouchKeyBit = i;
            break;
        }
    }
}

}  // namespace sead
