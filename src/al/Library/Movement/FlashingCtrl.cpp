#include "Library/Movement/FlashingCtrl.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace al {

/**
 * Constructs a controller that blinks an actor's model before a timer runs out.
 * @param pActor Actor to blink.
 * @param isHideModel Whether the model is hidden while blinking.
 * @param isPlaySe Whether blink and end sounds are played.
 */
FlashingCtrl::FlashingCtrl(LiveActor* pActor, bool isHideModel, bool isPlaySe)
    : mActor(pActor), mAudioKeeper(pActor), mIsHideModel(isHideModel), mIsPlaySe(isPlaySe) {}

/**
 * Counts down the timer and updates the blinking.
 */
void FlashingCtrl::movement() {
    if (mIsEnded) {
        return;
    }

    mTimer--;
    if (mTimer <= 0 || isClipped(mActor) || isDead(mActor)) {
        end();
        return;
    }

    if (isNowFlashing()) {
        updateFlashing();
    }
}

/**
 * Stops the timer and makes the model visible again.
 */
void FlashingCtrl::end() {
    mIsEnded = true;
    mTimer = 0;
    if (mIsHideModel && !isDead(mActor) && !isClipped(mActor) && isHideModel(mActor) &&
        isHideModel(mActor)) {
        showModel(mActor);
    }

    if (mIsPlaySe) {
        startSe(mAudioKeeper, "TimerEnd");
    }
}

/**
 * Checks whether the timer is in its blinking phase.
 * @return Whether the model is blinking.
 */
bool FlashingCtrl::isNowFlashing() const {
    return mTimer <= mFlashingStartTime;
}

/**
 * Shows or hides the model for the current blink phase.
 */
void FlashingCtrl::updateFlashing() {
    if (!mIsHideModel) {
        return;
    }

    bool isOn = isNowOn();
    bool isHidden = isHideModel(mActor);
    if (isOn) {
        if (!isHidden) {
            hideModel(mActor);
        }

        if (mIsHidden) {
            if (mIsPlaySe) {
                startSe(mAudioKeeper, "Blink");
            }

            mIsHidden = false;
        }
    } else {
        if (isHidden) {
            showModel(mActor);
        }

        mIsHidden = true;
    }
}

/**
 * Starts the timer.
 * @param time Timer length in steps.
 */
void FlashingCtrl::start(s32 time) {
    mIsEnded = false;
    mIsSlowInterval = false;
    mTimer = time;
    mFlashingStartTime = 180;
}

/**
 * Gets the current blink interval.
 * @return Blink interval in steps.
 */
s32 FlashingCtrl::getCurrentInterval() const {
    if (mIsSlowInterval) {
        return 8;
    }

    return isFastFlashing() ? 6 : 10;
}

/**
 * Gets the animation rate matching the blink interval.
 * @return Animation rate.
 */
f32 FlashingCtrl::getFlashingAnimRate() const {
    return 2.0f / getCurrentInterval();
}

/**
 * Checks whether a blink interval starts on this step.
 * @return Whether the model just blinked.
 */
bool FlashingCtrl::isNowJustFlashed() const {
    if (isNowFlashing() && mTimer % getCurrentInterval() == 0) {
        return true;
    }

    return false;
}

/**
 * Checks whether the timer is low enough for fast blinking.
 * @return Whether blinking is fast.
 */
bool FlashingCtrl::isFastFlashing() const {
    return mTimer < 90;
}

/**
 * Checks whether the model is in the hidden half of the blink cycle.
 * @return Whether the blink is on.
 */
bool FlashingCtrl::isNowOn() const {
    return (mTimer / getCurrentInterval()) % 2 == 0;
}

}  // namespace al
