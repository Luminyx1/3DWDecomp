#include "Player/PlayerActionConditionTimer.hpp"

/**
 * Holds once a condition has held for a number of frames in a row.
 * @param frame frames the condition has to hold
 * @param pCondition condition to time, or nullptr to just wait
 */
PlayerActionConditionTimer::PlayerActionConditionTimer(u32 frame, PlayerActionCondition* pCondition)
    : mCondition(pCondition), mFrame(frame) {}

/**
 * Counts the frames the condition held, restarting when it does not.
 * @return whether the condition has held for the whole time
 */
bool PlayerActionConditionTimer::check() {
    if (mCondition == nullptr || mCondition->check()) {
        if (mCounter < mFrame) {
            mCounter++;
        }
    } else {
        mCounter = 0;
    }

    return mCounter == mFrame;
}

/**
 * Sets up the timed condition and restarts the count.
 */
void PlayerActionConditionTimer::setup() {
    if (mCondition != nullptr) {
        mCondition->setup();
    }

    mCounter = 0;
}
