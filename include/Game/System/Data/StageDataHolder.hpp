#pragma once
#include "System/Data/StageUserData.hpp"
class GameDataHolder;
class StockItemList;
class CourseGreenStarInfo;
class StageDataHolder {
  public:
    explicit StageDataHolder(GameDataHolder* pHolder);
    void initializeData();
    void startStage(int courseId);
    void setGreenStarAcquireFlag(const CourseGreenStarInfo& rStars);
    void startTitle();
    void restartStage();
    void restartMysteryBox();
    void restartTimeupMysteryBox();
    void reenterStage();
    void resetStageScore();
    void gameOverStage();
    void clearStage(bool firstClear, bool newBestScore, bool newBestTime);
    void clearStageWorldWarp();
    int getStageBestScore() const;
    int getStageBestTime() const;
    int getTotalScore() const;
    int tryCalcLastStageBestScoreUserID() const;
    void resetStockItems();
    void acquireGreenStar(int starIndex);
    bool isAcquireGreenStar(int starIndex) const;
    void setCheckpointPass(int characterType);
    bool isContinuousMysteryBox() const;
    bool isRestartFromCheckpoint() const;
    bool isAcquireIllustItem() const;
    void setContinuousMysteryBox();
    void retireStage();
    void acquireIllustItem();
    void setUseAssistBlock();
    void resetCheckpointPass();
    bool isCheckpointPass() const;
    void setAcquireIllustItem(bool acquired);
    void setStagePlayerNumForAssistBlock(int count);
    int getStagePlayerNumForAssistBlock() const;
    void addTeamScore(int score);
    void addScore(int score, int userId);
    int getScore(int userId) const;
    bool isAlive(int userId) const;
    bool isGoalSuccess(int userId) const;
    float getGoalHeight(int userId) const;
    bool isGoalLeader(int userId) const;
    int getPlayerFigureType(int userId) const;
    void setAlive(int userId, bool alive);
    void setPlayerFigureType(int userId, int figureType);
    void setGoalState(int userId, bool success, float height, bool leader);
    bool stockItem(int itemId);
    void useStockItem();
    int getStockItem(int index) const;
    void setStampPickupCharType(int characterType);
    const CourseGreenStarInfo* getGreenStarAcquireFlag() const;
    int getCheckpointPassPlayerCharacter() const;
    void setStageTimerFrame(int frames);
    void addStageTimerFrame(int frames);
    void decStageTimerFrame(int frames);
    void countUpPlayTime(int frames);
    int calcStageTimerCount() const;
    int calcTimeAttackCount() const;
    static int calcStageTimerCountToFrame(int count);

  private:
    /**
     * @brief Clamp the stage timer to its supported range.
     * @param frames Requested number of timer frames.
     * @return A frame count from 0 through 43956.
     */
    static int clampTimer(int frames) {
        const int capped = frames < 43956 ? frames : 43956;
        return capped < 0 ? 0 : capped;
    }

    GameDataHolder* mpHolder;
    int mCourseId;
    bool mPlaying;
    bool mCleared;
    bool mWorldWarpClear;
    bool mFirstClear;
    bool mFirstStamp;
    bool mNewBestScore;
    bool mNewBestTime;
    bool mRestart;
    bool mMysteryBox;
    bool mRestartFromCheckpoint;
    bool mUnknown16;
    bool mRetired;
    bool mStampEntry;
    bool mStampCurrent;
    bool mStampCheckpoint;
    bool mAssistCurrent;
    bool mAssistCheckpoint;
    int mCheckpointCharacter;
    int mUnknown24;
    int mUnknown28;
    u64 mUnknown30;
    int mTimerFrames;
    int mPlayFrames;
    int mTeamScore;
    int mAssistPlayerCount;
    StageUserData* mpUsers;
    StockItemList* mpStockItems;
    CourseGreenStarInfo* mpStarsCurrent;
    CourseGreenStarInfo* mpStarsCheckpoint;
    int mStampCharacter;
};
static_assert(sizeof(StageDataHolder) == 0x70);
