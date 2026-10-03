#include "System/ControlUserData.hpp"
#include "Util/PlayerUtil.hpp"

/**
 * @brief Creates an inactive user with no assigned controller or character.
 */
ControlUserData::ControlUserData() : mFigureType(rc::getPlayerFigureTypeDefault()) {}

/**
 * @brief Checks whether the user is participating, including a player who has died.
 * @return True for the alive and dead-in-stage states.
 */
bool ControlUserData::isActive() const { return mState < Deactive; }

/**
 * @brief Checks whether the participating player has died in the current stage.
 * @return True when the state is dead-in-stage.
 */
bool ControlUserData::isDeadInStage() const { return mState == DeadInStage; }

/**
 * @brief Restores the user to the alive state.
 */
void ControlUserData::resetDeadInStage() { mState = Alive; }

/**
 * @brief Checks whether the user is not participating.
 * @return True when the state is inactive.
 */
bool ControlUserData::isDeactive() const { return mState == Deactive; }

/**
 * @brief Marks the user as having died in the current stage.
 */
void ControlUserData::setDeadInStage() { mState = DeadInStage; }
