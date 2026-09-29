#include "Player/PlayerActionConditionRaccoonDogFall1st.hpp"
#include "Player/IUsePlayerRaccoonDogFallTask.hpp"

/**
 * Holds during the first tanooki slow fall after leaving the ground.
 * @param pFallTask tanooki fall task
 */
PlayerActionConditionRaccoonDogFall1st::PlayerActionConditionRaccoonDogFall1st(const IUsePlayerRaccoonDogFallTask* pFallTask) : mFallTask(pFallTask) {}

/**
 * @return whether this is the first slow fall
 */
bool PlayerActionConditionRaccoonDogFall1st::check() {
    return mFallTask->isFirstFalling();
}
