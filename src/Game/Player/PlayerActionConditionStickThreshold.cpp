#include "Player/PlayerActionConditionStickThreshold.hpp"
#include <math/seadMathCalcCommon.h>
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionArg.hpp"

/**
 * Holds while the stick is tilted far.
 * @param pArg the player's systems
 * @param pInput the player's input
 */
PlayerActionConditionStickThreshold::PlayerActionConditionStickThreshold(const PlayerActionArg* pArg,
                                                                         const IUsePlayerInput* pInput)
    : mArg(pArg), mInput(pInput) {}

/**
 * @return whether the stick is tilted more than 90%
 */
bool PlayerActionConditionStickThreshold::check() {
    const sead::Vector3f& rMoveVec = mArg->getInput()->getMoveVec();
    return sead::Mathf::sqrt(rMoveVec.x * rMoveVec.x + rMoveVec.z * rMoveVec.z) > 0.9f;
}
