#include "Player/PlayerActionConditionAfterPunch.hpp"
#include "Player/Normal/PlayerCounterAfterPunch.hpp"

/**
 * Holds once enough frames passed since the last punch.
 * @param pCounter frames since the last punch
 * @param frame frames to wait
 */
PlayerActionConditionAfterPunch::PlayerActionConditionAfterPunch(const PlayerCounterAfterPunch* pCounter, u32 frame) : mCounter(pCounter), mFrame(frame) {}

/**
 * @return whether enough frames passed
 */
bool PlayerActionConditionAfterPunch::check() {
    return mCounter->getCounter() >= mFrame;
}
