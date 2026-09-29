#include "Player/PlayerActionConditionBigTrig.hpp"
#include "Player/IUsePlayerSizeTrigger.hpp"

/**
 * Holds on the frame the player grows big.
 * @param pSizeTrigger size change triggers
 */
PlayerActionConditionBigTrig::PlayerActionConditionBigTrig(const IUsePlayerSizeTrigger* pSizeTrigger) : mSizeTrigger(pSizeTrigger) {}

/**
 * @return whether the player just grew big
 */
bool PlayerActionConditionBigTrig::check() {
    return mSizeTrigger->isBigTrig();
}
