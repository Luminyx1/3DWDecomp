#include "Player/PlayerActionConditionFlag.hpp"
#include "Player/IUsePlayerFlag.hpp"

/**
 * Holds when a flag has the expected value.
 * @param pFlag flag to test
 * @param pIsOn value to test for
 */
PlayerActionConditionFlag::PlayerActionConditionFlag(const IUsePlayerFlag* pFlag, bool pIsOn) : mFlag(pFlag), mIsOn(pIsOn) {}

/**
 * @return whether the flag has the expected value
 */
bool PlayerActionConditionFlag::check() {
    return mFlag->isOn() == mIsOn;
}
