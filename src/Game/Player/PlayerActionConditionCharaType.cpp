#include "Player/PlayerActionConditionCharaType.hpp"
#include "Player/IUsePlayerCharaQuery.hpp"

/**
 * Holds when the player is a given character.
 * @param pCharaQuery character query
 * @param pChara character to test for
 */
PlayerActionConditionCharaType::PlayerActionConditionCharaType(const IUsePlayerCharaQuery* pCharaQuery, EPlayerChara pChara) : mCharaQuery(pCharaQuery), mChara(pChara) {}

/**
 * @return whether the player is that character
 */
bool PlayerActionConditionCharaType::check() {
    return mCharaQuery->isChara(mChara);
}
