#include "System/Data/StageUserData.hpp"
#include "Util/PlayerUtil.hpp"

/**
 * @brief Creates a stage record with the default figure and empty progress.
 */
StageUserData::StageUserData() { init(); }

/**
 * @brief Restores the default figure and clears all per-stage progress.
 */
void StageUserData::init() {
    mScore = 0;
    mFigureType = rc::getPlayerFigureTypeDefault();
    clearStageResult();
}

/**
 * @brief Adds a score adjustment to the stage total.
 * @param score Signed score adjustment; the caller must keep the total representable.
 */
void StageUserData::addScore(s32 score) { mScore += score; }

/**
 * @brief Clears score and stage result flags while retaining the player's figure.
 */
void StageUserData::resetScore() {
    mScore = 0;
    clearStageResult();
}

/**
 * @brief Records the player's current transformation.
 * @param figureType Player figure identifier; stored without validation.
 */
void StageUserData::setFigureType(s32 figureType) { mFigureType = figureType; }

/**
 * @brief Updates the player's alive flag.
 * @param alive Whether the player is alive in the stage.
 */
void StageUserData::setAlive(bool alive) { mAlive = alive; }

/**
 * @brief Records the player's goal result and its associated flags.
 * @param reachedGoal Whether the player reached the goal.
 * @param goalHeight Player height on the goal pole; stored without validation.
 * @param goalLeader Whether the player is the goal leader.
 */
void StageUserData::setGoalState(bool reachedGoal, f32 goalHeight, bool goalLeader) {
    mReachedGoal = reachedGoal;
    mGoalLeader = goalLeader;
    mGoalHeight = goalHeight;
}
