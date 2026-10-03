#include "System/Data/StageDataHolder.hpp"
#include "System/Data/StockItemList.hpp"

/**
 * @brief Read the current stage state.
 * @return True when the corresponding stage state is set.
 */
bool StageDataHolder::isContinuousMysteryBox() const { return mMysteryBox; }

/**
 * @brief Read the current stage state.
 * @return True when the corresponding stage state is set.
 */
bool StageDataHolder::isRestartFromCheckpoint() const { return mRestartFromCheckpoint; }

/**
 * @brief Read the current stage state.
 * @return True when the corresponding stage state is set.
 */
bool StageDataHolder::isAcquireIllustItem() const { return mStampCurrent; }

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::setContinuousMysteryBox() { mMysteryBox = true; }

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::retireStage() {
    mPlaying = false;
    mRetired = true;
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::acquireIllustItem() {
    mStampCurrent = true;
    mStampCheckpoint = true;
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::setUseAssistBlock() {
    mAssistCurrent = true;
    mAssistCheckpoint = true;
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::resetCheckpointPass() { mCheckpointCharacter = -1; }

/**
 * @brief Check whether a checkpoint has been saved.
 * @return True when a checkpoint character is recorded.
 */
bool StageDataHolder::isCheckpointPass() const { return mCheckpointCharacter != -1; }

/**
 * @brief Set stamp ownership at stage entry and both restart states.
 * @param acquired Whether the stage stamp is already owned.
 */
void StageDataHolder::setAcquireIllustItem(bool acquired) {
    mStampEntry = acquired;
    mStampCurrent = acquired;
    mStampCheckpoint = acquired;
}

/**
 * @brief Keep the minimum positive player count for assist-block eligibility.
 * @param count Current number of stage players; normally 1 through 4.
 */
void StageDataHolder::setStagePlayerNumForAssistBlock(int count) {
    if (mAssistPlayerCount < 1) {
        mAssistPlayerCount = count;
    } else {
        mAssistPlayerCount = mAssistPlayerCount < count ? mAssistPlayerCount : count;
    }
}

/**
 * @brief Read the assist-block player count.
 * @return The minimum recorded count, or -1 before initialization.
 */
int StageDataHolder::getStagePlayerNumForAssistBlock() const { return mAssistPlayerCount; }

/**
 * @brief Add points shared by the entire team.
 * @param score Signed points to add without clamping.
 */
void StageDataHolder::addTeamScore(int score) { mTeamScore += score; }

/**
 * @brief Add points to one player.
 * @param score Signed points to add.
 * @param userId Control-user index within the allocated stage-user array.
 */
void StageDataHolder::addScore(int score, int userId) { mpUsers[userId].addScore(score); }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
int StageDataHolder::getScore(int userId) const { return mpUsers[userId].mScore; }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
bool StageDataHolder::isAlive(int userId) const { return mpUsers[userId].mAlive; }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
bool StageDataHolder::isGoalSuccess(int userId) const { return mpUsers[userId].mReachedGoal; }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
float StageDataHolder::getGoalHeight(int userId) const { return mpUsers[userId].mGoalHeight; }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
bool StageDataHolder::isGoalLeader(int userId) const { return mpUsers[userId].mGoalLeader; }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
int StageDataHolder::getPlayerFigureType(int userId) const { return mpUsers[userId].mFigureType; }

/**
 * @brief Update a player's stage state.
 * @param userId Control-user index within the allocated stage-user array.
 * @param alive New value to store in the player record.
 */
void StageDataHolder::setAlive(int userId, bool alive) { mpUsers[userId].setAlive(alive); }

/**
 * @brief Update a player's stage state.
 * @param userId Control-user index within the allocated stage-user array.
 * @param figureType New value to store in the player record.
 */
void StageDataHolder::setPlayerFigureType(int userId, int figureType) {
    mpUsers[userId].setFigureType(figureType);
}

/**
 * @brief Record a player's goal result.
 * @param userId Control-user index within the allocated stage-user array.
 * @param success Whether the player successfully reached the goal.
 * @param height Player height on the goal pole.
 * @param leader Whether the player is the goal leader.
 */
void StageDataHolder::setGoalState(int userId, bool success, float height, bool leader) {
    mpUsers[userId].setGoalState(success, height, leader);
}

/**
 * @brief Add an item to the stage stock.
 * @param itemId Stock-item identifier; zero is rejected.
 * @return True when the stock operation is accepted.
 */
bool StageDataHolder::stockItem(int itemId) { return mpStockItems->stockItem(itemId); }

/**
 * @brief Consume the first stocked item.
 */
void StageDataHolder::useStockItem() { mpStockItems->useStockItem(); }

/**
 * @brief Read a stocked item.
 * @param index Slot index from 0 through 3.
 * @return The item identifier in that slot.
 */
int StageDataHolder::getStockItem(int index) const { return mpStockItems->getStockItem(index); }

/**
 * @brief Remember which character collected the stamp.
 * @param characterType Character identifier of the player collecting the stamp.
 */
void StageDataHolder::setStampPickupCharType(int characterType) { mStampCharacter = characterType; }

/**
 * @brief Access the current green-star flags.
 * @return The current stage green-star record.
 */
const CourseGreenStarInfo* StageDataHolder::getGreenStarAcquireFlag() const { return mpStarsCurrent; }

/**
 * @brief Read the checkpoint character.
 * @return The saved character identifier, or -1 for no checkpoint.
 */
int StageDataHolder::getCheckpointPassPlayerCharacter() const { return mCheckpointCharacter; }

/**
 * @brief Update the stage timer within its allowed range.
 * @param frames Signed frame count; the resulting timer is clamped to 0 through 43956.
 */
void StageDataHolder::setStageTimerFrame(int frames) { mTimerFrames = clampTimer(frames); }

/**
 * @brief Update the stage timer within its allowed range.
 * @param frames Signed frame count; the resulting timer is clamped to 0 through 43956.
 */
void StageDataHolder::addStageTimerFrame(int frames) { mTimerFrames = clampTimer(mTimerFrames + frames); }

/**
 * @brief Update the stage timer within its allowed range.
 * @param frames Signed frame count; the resulting timer is clamped to 0 through 43956.
 */
void StageDataHolder::decStageTimerFrame(int frames) {
    mTimerFrames = clampTimer(mTimerFrames - frames);
    mPlayFrames += frames;
}

/**
 * @brief Accumulate frames spent playing the stage.
 * @param frames Signed number of elapsed frames to add.
 */
void StageDataHolder::countUpPlayTime(int frames) { mPlayFrames += frames; }

/**
 * @brief Convert remaining frames to displayed timer counts.
 * @return Remaining timer counts, rounded upward.
 */
int StageDataHolder::calcStageTimerCount() const { return (mTimerFrames + 43) / 44; }

/**
 * @brief Convert play time to a bounded time-attack count.
 * @return Elapsed timer counts clamped to 0 through 999.
 */
int StageDataHolder::calcTimeAttackCount() const {
    const int count = mPlayFrames / 44;
    return count < 0 ? 0 : count > 999 ? 999 : count;
}

/**
 * @brief Convert timer counts to frames.
 * @param count Timer count; multiplication must fit in a signed integer.
 * @return The corresponding frame count.
 */
int StageDataHolder::calcStageTimerCountToFrame(int count) { return count * 44; }

#include "System/Data/WorldInfo.hpp"

/**
 * @brief Create an empty world-stage list.
 */
WorldInfo::WorldInfo() : mStageCount(0), mpStages(nullptr) {}

/**
 * @brief Bind a world to its stage records.
 * @param worldId One-based world identifier.
 * @param pStages Array containing at least count stage records.
 * @param count Number of stage records in the world.
 */
void WorldInfo::init(int worldId, StageDatabaseInfo* pStages, int count) {
    mWorldId = worldId;
    mpStages = pStages;
    mStageCount = count;
}

/**
 * @brief Access a stage record within the world.
 * @param index Zero-based stage index below mStageCount.
 * @return The selected stage record.
 */
StageDatabaseInfo* WorldInfo::getStageInfoByIndex(int index) const { return &mpStages[index]; }

/**
 * @brief Allocate world and stage database records.
 * @param worldCount Nonnegative number of worlds.
 * @param stageCount Nonnegative number of stage records.
 */
WorldInfoList::WorldInfoList(int worldCount, int stageCount) : mWorldCount(worldCount) {
    mpWorlds = new WorldInfo[worldCount];
    mStageCount = stageCount;
    mpStages = new StageDatabaseInfo[stageCount];
}

/**
 * @brief Access a world record by its one-based identifier.
 * @param worldId World identifier from 1 through mWorldCount.
 * @return The selected world record.
 */
WorldInfo* WorldInfoList::findWorldInfo(int worldId) const { return &mpWorlds[worldId - 1]; }

/**
 * @brief Access the flat stage database.
 * @param index Zero-based index below mStageCount.
 * @return The selected stage record.
 */
StageDatabaseInfo* WorldInfoList::getStageInfoByIndex(int index) const { return &mpStages[index]; }

/**
 * @brief Search every world for a course identifier.
 * @param courseId Course identifier to locate.
 * @return The matching stage record, or nullptr when absent.
 */
StageDatabaseInfo* WorldInfoList::findStageDatabaseInfo(int courseId) const {
    for (int i = 0; i < mWorldCount; ++i) {
        const auto& rWorld = mpWorlds[i];
        for (int j = 0; j < rWorld.mStageCount; ++j) {
            auto* pStage = &rWorld.mpStages[j];
            if (pStage->getCourseId() == courseId) {
                return pStage;
            }
        }
    }
    return nullptr;
}
