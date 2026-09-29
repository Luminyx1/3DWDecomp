#include "controller/seadControllerBase.h"

namespace sead
{
const f32 ControllerBase::cStickHoldThresholdDefault = 0.5f;
const f32 ControllerBase::cStickReleaseThresholdDefault = 0.25f;

const Vector2f ControllerBase::cInvalidPointer(Mathf::minNumber(), Mathf::minNumber());
const Vector2i ControllerBase::cInvalidPointerS32(Mathi::minNumber(), Mathi::minNumber());

/**
 * Sets up the pad layout and resets all input state.
 * @param padBitMax number of pad bits in use
 * @param leftStickCrossStartBit first pad bit of the emulated left stick cross, or -1
 * @param rightStickCrossStartBit first pad bit of the emulated right stick cross, or -1
 * @param touchKeyBit pad bit that mirrors the pointer, or -1
 */
ControllerBase::ControllerBase(s32 padBitMax, s32 leftStickCrossStartBit,
                               s32 rightStickCrossStartBit, s32 touchKeyBit)
    : mPadTrig(), mPadRelease(), mPadRepeat(), mPointerFlag(), mPointerS32(cInvalidPointerS32),
      mPointerBound(), mLeftStickHoldThreshold(0.5f), mRightStickHoldThreshold(0.5f),
      mLeftStickReleaseThreshold(0.25f), mRightStickReleaseThreshold(0.25f), mPadBitMax(padBitMax),
      mLeftStickCrossStartBit(leftStickCrossStartBit),
      mRightStickCrossStartBit(rightStickCrossStartBit), mTouchKeyBit(touchKeyBit), mIdleFrame(0),
      mPadHold(), mPointer(cInvalidPointer), mLeftStick(0.0f, 0.0f), mRightStick(0.0f, 0.0f),
      mLeftAnalogTrigger(0.0f), mRightAnalogTrigger(0.0f)
{
    if (cPadIdx_MaxBase < padBitMax)
    {
        mPadBitMax = cPadIdx_MaxBase;
    }

    for (u32 i = 0; i < cPadIdx_MaxBase; i++)
    {
        mPadRepeatDelays[i] = 30;
        mPadRepeatPulses[i] = 1;
        mPadHoldCounts[i] = 0;
    }
}

/**
 * Updates the pointer, clipping it against the pointer bound if one is set.
 * @param isOn whether the pointer is currently on
 * @param touchkeyHold whether the touch key bit should be held
 * @param rPos raw pointer position
 */
void ControllerBase::setPointerWithBound_(bool isOn, bool touchkeyHold, const Vector2f& rPos)
{
    if (isOn)
    {
        if (!mPointerBound.isUndef())
        {
            if (mPointerBound.isInside(rPos))
            {
                mPointer.x = rPos.x - mPointerBound.getMin().x;
                mPointer.y = rPos.y - mPointerBound.getMin().y;
            }
            else
            {
                isOn = false;
            }
        }
        else
        {
            mPointer.x = rPos.x;
            mPointer.y = rPos.y;
        }
    }

    mPointerFlag.change(cPointerOn, isOn);

    if (mTouchKeyBit >= 0)
    {
        mPadHold.changeBit(mTouchKeyBit, isOn && touchkeyHold);
    }

    if (mPointerFlag.isOn(cPointerUnkFlag3))
    {
        if (mPointerFlag.isOff(cPointerOn))
        {
            mPointer.x = cInvalidPointer.x;
            mPointer.y = cInvalidPointer.y;
        }

        mPointerFlag.reset(cPointerUnkFlag3);
    }
}

/**
 * Derives stick cross bits, trigger/release/repeat masks and pointer edges from the hold state.
 * @param prevHold hold mask of the previous frame
 * @param prevPointerOn whether the pointer was on in the previous frame
 */
void ControllerBase::updateDerivativeParams_(u32 prevHold, bool prevPointerOn)
{
    u32 stick_hold = 0;

    if (mLeftStickCrossStartBit >= 0)
    {
        stick_hold |= getStickHold_(prevHold, mLeftStick, mLeftStickHoldThreshold,
                                    mLeftStickReleaseThreshold, mLeftStickCrossStartBit);
    }

    if (mRightStickCrossStartBit >= 0)
    {
        stick_hold |= getStickHold_(prevHold, mRightStick, mRightStickHoldThreshold,
                                    mRightStickReleaseThreshold, mRightStickCrossStartBit);
    }

    mPadHold.setDirect((mPadHold.getDirect() & ~createStickCrossMask_()) | stick_hold);

    mPadTrig.setDirect(~prevHold & mPadHold.getDirect());
    mPadRelease.setDirect(prevHold & ~mPadHold.getDirect());
    mPadRepeat.setDirect(0);

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        if (mPadHold.isOnBit(i))
        {
            if (mPadRepeatPulses[i])
            {
                if (mPadHoldCounts[i] == mPadRepeatDelays[i])
                {
                    mPadRepeat.setBit(i);
                }

                else if (mPadRepeatDelays[i] < mPadHoldCounts[i] &&
                         (mPadHoldCounts[i] - mPadRepeatDelays[i]) % mPadRepeatPulses[i] == 0)
                {
                    mPadRepeat.setBit(i);
                }
            }

            mPadHoldCounts[i]++;
        }
        else
        {
            mPadHoldCounts[i] = 0;
        }
    }

    mPointerFlag.change(cPointerOnNow, !prevPointerOn && mPointerFlag.isOn(cPointerOn));
    mPointerFlag.change(cPointerOffNow, prevPointerOn && mPointerFlag.isOff(cPointerOn));

    mPointerS32.x = (s32)mPointer.x;
    mPointerS32.y = (s32)mPointer.y;
}

/**
 * Gets how many frames a pad bit has been held.
 * @param bit pad bit
 * @return number of frames held
 */
u32 ControllerBase::getPadHoldCount(s32 bit) const
{
    return mPadHoldCounts[bit];
}

/**
 * Sets the repeat delay and interval for the given pad bits.
 * @param mask pad bits to change
 * @param delayFrame frames before the first repeat
 * @param pulseFrame frames between repeats
 */
void ControllerBase::setPadRepeat(u32 mask, u8 delayFrame, u8 pulseFrame)
{
    BitFlag32 pad_to_set(mask);

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        if (pad_to_set.isOnBit(i))
        {
            mPadRepeatDelays[i] = delayFrame;
            mPadRepeatPulses[i] = pulseFrame;
        }
    }
}

/**
 * Sets the left stick cross hold and release thresholds.
 * @param hold stick length needed to press a direction
 * @param release stick length below which a direction is released
 */
void ControllerBase::setLeftStickCrossThreshold(f32 hold, f32 release)
{
    if (hold >= release)
    {
        mLeftStickHoldThreshold = hold;
        mLeftStickReleaseThreshold = release;
    }
}

/**
 * Sets the right stick cross hold and release thresholds.
 * @param hold stick length needed to press a direction
 * @param release stick length below which a direction is released
 */
void ControllerBase::setRightStickCrossThreshold(f32 hold, f32 release)
{
    if (hold >= release)
    {
        mRightStickHoldThreshold = hold;
        mRightStickReleaseThreshold = release;
    }
}

/**
 * Sets the area the pointer is limited to.
 * @param rBound pointer bound
 */
void ControllerBase::setPointerBound(const BoundBox2f& rBound)
{
    mPointerBound.set(rBound.getMin(), rBound.getMax());
    mPointerFlag.set(cPointerUnkFlag3);
}

/**
 * Converts a stick position to cross direction bits with hysteresis.
 * @param prevHold hold mask of the previous frame
 * @param rStick stick position
 * @param holdThreshold stick length needed to press a direction
 * @param releaseThreshold stick length below which directions are released
 * @param startBit first pad bit of the cross
 * @return pad bits of the held directions
 */
u32 ControllerBase::getStickHold_(u32 prevHold, const Vector2f& rStick, f32 holdThreshold,
                                  f32 releaseThreshold, s32 startBit)
{
    f32 length = rStick.length();

    if (length < releaseThreshold ||
        (length < holdThreshold &&
         (prevHold & (1 << (startBit + cCrossUp) | 1 << (startBit + cCrossDown) |
                      1 << (startBit + cCrossLeft) | 1 << (startBit + cCrossRight))) == 0))
    {
        return 0;
    }
    else
    {
        u32 angle = Mathf::atan2Idx(rStick.y, rStick.x);

        if (angle < 0x10000000)
        {
            return 1 << (startBit + cCrossRight);
        }
        else if (angle < 0x30000000)
        {
            return 1 << (startBit + cCrossRight) | 1 << (startBit + cCrossUp);
        }
        else if (angle < 0x50000000)
        {
            return 1 << (startBit + cCrossUp);
        }
        else if (angle < 0x70000000)
        {
            return 1 << (startBit + cCrossLeft) | 1 << (startBit + cCrossUp);
        }
        else if (angle < 0x90000000)
        {
            return 1 << (startBit + cCrossLeft);
        }
        else if (angle < 0xb0000000)
        {
            return 1 << (startBit + cCrossLeft) | 1 << (startBit + cCrossDown);
        }
        else if (angle < 0xd0000000)
        {
            return 1 << (startBit + cCrossDown);
        }
        else if (angle < 0xf0000000)
        {
            return 1 << (startBit + cCrossRight) | 1 << (startBit + cCrossDown);
        }
        else
        {
            return 1 << (startBit + cCrossRight);
        }
    }
}

/**
 * Checks whether no input is active.
 * @return true if nothing is held, touched, tilted or pulled
 */
bool ControllerBase::isIdleBase_()
{
    return getHoldMask() == 0 && mPointerFlag.isOff(1) && mLeftStick.isZero() &&
           mRightStick.isZero() && mLeftAnalogTrigger == 0.0f && mRightAnalogTrigger == 0.0f;
}

/**
 * Clears all input state.
 */
void ControllerBase::setIdleBase_()
{
    mPadHold.makeAllZero();
    mPadTrig.makeAllZero();
    mPadRelease.makeAllZero();
    mPadRepeat.makeAllZero();
    mPointerFlag.makeAllZero();

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        mPadHoldCounts[i] = 0;
    }

    mPointer = cInvalidPointer;
    mPointerS32 = cInvalidPointerS32;
    mLeftStick.set(0.0f, 0.0f);
    mRightStick.set(0.0f, 0.0f);
    mLeftAnalogTrigger = 0.0f;
    mRightAnalogTrigger = 0.0f;
}

/**
 * Builds the mask of all pad bits used by the emulated stick crosses.
 * @return stick cross pad bits
 */
u32 ControllerBase::createStickCrossMask_()
{
    BitFlag32 mask;

    if (mLeftStickCrossStartBit >= 0)
    {
        mask.setBit(mLeftStickCrossStartBit + cCrossUp);
        mask.setBit(mLeftStickCrossStartBit + cCrossDown);
        mask.setBit(mLeftStickCrossStartBit + cCrossLeft);
        mask.setBit(mLeftStickCrossStartBit + cCrossRight);
    }

    if (mRightStickCrossStartBit >= 0)
    {
        mask.setBit(mRightStickCrossStartBit + cCrossUp);
        mask.setBit(mRightStickCrossStartBit + cCrossDown);
        mask.setBit(mRightStickCrossStartBit + cCrossLeft);
        mask.setBit(mRightStickCrossStartBit + cCrossRight);
    }

    return mask.getDirect();
}

}  // namespace sead
