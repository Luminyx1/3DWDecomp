#pragma once

#include <basis/seadTypes.h>

class StageUserData {
  public:
    StageUserData();
    void init();
    void addScore(s32 score);
    void resetScore();
    void setFigureType(s32 figureType);
    void setAlive(bool alive);
    void setGoalState(bool reachedGoal, f32 goalHeight, bool goalLeader);

    s32 mScore;
    s32 mFigureType;
    bool mAlive;
    bool mReachedGoal;
    bool mGoalLeader;
    f32 mGoalHeight;

  private:
    /**
     * @brief Clears alive and goal results without changing score or figure type.
     */
    void clearStageResult() {
        mAlive = false;
        mReachedGoal = false;
        mGoalLeader = false;
        mGoalHeight = 0.0f;
    }
};

static_assert(sizeof(StageUserData) == 0x10);
