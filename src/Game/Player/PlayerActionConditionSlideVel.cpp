#include "Player/PlayerActionConditionSlideVel.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/Normal/PlayerProperty.hpp"
#include "Player/PlayerActionFunc.hpp"

/**
 * Holds while the player moves down the slope they stand on.
 * @param pProperty the player's physical state
 * @param pCollision the player's collision
 */
PlayerActionConditionSlideVel::PlayerActionConditionSlideVel(const PlayerProperty* pProperty,
                                                             const IUsePlayerCollision* pCollision)
    : mProperty(pProperty), mCollision(pCollision) {}

/**
 * @return whether the velocity points down the floor's slope
 */
bool PlayerActionConditionSlideVel::check() {
    if (!mCollision->isOnFloor()) {
        return false;
    }

    IUsePlayerCollision::Info info = {};
    mCollision->getFloorInfo(&info);
    sead::Vector3f downward;
    PlayerActionFunc::calcDownward(&downward, mProperty, info.mNormal);
    return !(downward.dot(mProperty->getVelocity()) <= 0.0f);
}
