#include "System/Data/StageDataHolder.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "System/CourseInfo.hpp"
#include "System/Data/StockItemList.hpp"
#include "System/Data/WorldInfo.hpp"
#include "System/GameDataFile.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataHolder.hpp"
#include "Util/ControlUserUtil.hpp"

/**
 * @brief Allocate per-stage records and reset all stage state.
 * @param pHolder Owning game-data holder; must outlive this object.
 */
StageDataHolder::StageDataHolder(GameDataHolder* pHolder)
    : mpHolder(pHolder), mCourseId(GameDataFunction::getInvalidCourseId()), mPlaying(false),
      mCleared(false), mWorldWarpClear(false), mFirstClear(false), mFirstStamp(false),
      mNewBestScore(false), mNewBestTime(false), mRestart(false), mMysteryBox(false),
      mRestartFromCheckpoint(false), mUnknown16(false), mRetired(false), mStampEntry(false),
      mStampCurrent(false), mStampCheckpoint(false), mAssistCurrent(false),
      mAssistCheckpoint(false), mCheckpointCharacter(-1), mUnknown24(-1), mUnknown28(0),
      mUnknown30(0), mTimerFrames(0), mPlayFrames(0), mTeamScore(0), mAssistPlayerCount(-1),
      mpUsers(nullptr), mpStockItems(nullptr), mpStarsCurrent(nullptr),
      mpStarsCheckpoint(nullptr), mStampCharacter(-1) {
    mpStarsCurrent = new CourseGreenStarInfo();
    mpStarsCheckpoint = new CourseGreenStarInfo();
    mpUsers = new StageUserData[rc::getControlUserNumMax()];
    mpStockItems = new StockItemList(pHolder);
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::setContinuousMysteryBox() { mMysteryBox = true; }

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
 * @brief Reset the stage state, keeping persistent fields across restarts.
 */
void StageDataHolder::initializeData() {
    if (!mRestart) {
        mUnknown24 = -1;
        mUnknown28 = 0;
        mUnknown30 = 0;
    }

    mCourseId = GameDataFunction::getInvalidCourseId();
    mCleared = false;
    mWorldWarpClear = false;
    mFirstClear = false;
    mFirstStamp = false;
    mNewBestScore = false;
    mNewBestTime = false;
    mRestart = false;
    mMysteryBox = false;
    mRestartFromCheckpoint = false;
    mRetired = false;
    mStampEntry = false;
    mStampCurrent = false;
    mStampCheckpoint = false;
    mAssistCurrent = false;
    mAssistCheckpoint = false;
    mCheckpointCharacter = -1;
    mpStarsCurrent->initialize();
    mpStarsCheckpoint->initialize();
    mTimerFrames = 0;
    mPlayFrames = 0;
    mTeamScore = 0;
    mAssistPlayerCount = -1;
    mStampCharacter = -1;
    for (int i = 0; i < rc::getControlUserNumMax(); ++i) {
        mpUsers[i].init();
    }

    mpStockItems->initialize();
}

/**
 * @brief Begin playing a course from its saved progress.
 * @param courseId Course identifier to start.
 */
void StageDataHolder::startStage(int courseId) {
    mPlaying = true;
    initializeData();
    mCourseId = courseId;
    GameDataFile* pFile = mpHolder->getPlayingFile();
    const CourseInfo* pCourse = pFile->getCourseInfo(courseId);
    setGreenStarAcquireFlag(pCourse->getGreenStarInfo());
    setAcquireIllustItem(pCourse->isAcquireIllustItem());
    mpStockItems->copy(pFile->getStockItemList());
    mNewBestScore = false;
    mNewBestTime = false;
    mStampCharacter = -1;
}

/**
 * @brief Set the current and checkpoint green-star flags.
 * @param rStars Green-star flags to copy.
 */
void StageDataHolder::setGreenStarAcquireFlag(const CourseGreenStarInfo& rStars) {
    mpStarsCheckpoint->copy(rStars);
    mpStarsCurrent->copy(rStars);
}

/**
 * @brief Set stamp ownership at stage entry and both restart states.
 * @param acquired Whether the stage stamp is already owned.
 */
void StageDataHolder::setAcquireIllustItem(bool acquired) {
    mStampCheckpoint = acquired;
    mStampCurrent = acquired;
    mStampEntry = acquired;
}

/**
 * @brief Reset the stage state for the title screen.
 */
void StageDataHolder::startTitle() {
    mPlaying = true;
    initializeData();
}

/**
 * @brief Restart the stage from the last checkpoint state.
 */
void StageDataHolder::restartStage() {
    mPlaying = true;
    mpStarsCurrent->copy(*mpStarsCheckpoint);
    mStampCurrent = mStampCheckpoint;
    mAssistCurrent = mAssistCheckpoint;
    mTeamScore = 0;
    for (int i = 0; i < rc::getControlUserNumMax(); ++i) {
        mpUsers[i].init();
    }

    mPlayFrames = 0;
    mRestartFromCheckpoint = isCheckpointPass();
    mTimerFrames = 0;
    mRestart = true;
    mStampCharacter = -1;
    mpStockItems->retireStage();
}

/**
 * @brief Check whether a checkpoint has been saved.
 * @return True when a checkpoint character is recorded.
 */
bool StageDataHolder::isCheckpointPass() const { return mCheckpointCharacter != -1; }

/**
 * @brief Restart a mystery-box stage.
 */
void StageDataHolder::restartMysteryBox() {
    mPlaying = true;
    restartStage();
    mpStockItems->retireStage();
}

/**
 * @brief Restart a mystery-box stage after a time-up, restoring the saved stock items.
 */
void StageDataHolder::restartTimeupMysteryBox() {
    mPlaying = true;
    restartStage();
    resetStockItems();
}

/**
 * @brief Start the current course again from its saved progress.
 */
void StageDataHolder::reenterStage() {
    const bool restart = !mMysteryBox;
    startStage(mCourseId);
    mRestart = restart;
}

/**
 * @brief Clear the team and player scores and the elapsed play time.
 */
void StageDataHolder::resetStageScore() {
    mTeamScore = 0;
    for (int i = 0; i < rc::getControlUserNumMax(); ++i) {
        mpUsers[i].resetScore();
    }

    mPlayFrames = 0;
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::retireStage() {
    mPlaying = false;
    mRetired = true;
}

/**
 * @brief Leave the stage after a game over.
 */
void StageDataHolder::gameOverStage() {
    mPlaying = false;
    initializeData();
}

/**
 * @brief Record a regular stage clear.
 * @param firstClear Whether this is the course's first clear.
 * @param newBestScore Whether the clear set a new best score.
 * @param newBestTime Whether the clear set a new best time.
 */
void StageDataHolder::clearStage(bool firstClear, bool newBestScore, bool newBestTime) {
    mPlaying = false;
    mCleared = true;
    mWorldWarpClear = false;
    mFirstClear = firstClear;
    mFirstStamp = !mStampEntry && mStampCurrent;
    mNewBestScore = newBestScore;
    mNewBestTime = newBestTime;
    mRetired = false;
    mpStockItems->clearStage();
}

/**
 * @brief Record a stage clear through a world warp.
 */
void StageDataHolder::clearStageWorldWarp() {
    mPlaying = false;
    mCleared = true;
    mWorldWarpClear = true;
    mFirstClear = false;
    mFirstStamp = !mStampEntry && mStampCurrent;
    mNewBestScore = false;
    mNewBestTime = false;
    mRetired = false;
}

/**
 * @brief Read the saved best score of the current course.
 * @return The best score.
 */
int StageDataHolder::getStageBestScore() const {
    return mpHolder->getCourseInfo(mCourseId)->getBestScore();
}

/**
 * @brief Read the saved best time of the current course.
 * @return The best time.
 */
int StageDataHolder::getStageBestTime() const {
    return mpHolder->getCourseInfo(mCourseId)->getBestTime();
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
 * @brief Sum the team score and all player scores.
 * @return The total score, capped at 999999.
 */
int StageDataHolder::getTotalScore() const {
    int total = mTeamScore;
    for (int i = 0; i < rc::getControlUserNumMax(); ++i) {
        total += mpUsers[i].mScore;
    }

    return total < 999999 ? total : 999999;
}

/**
 * @brief Find the active player with the unique highest score.
 * @return The player's user ID, or -1 when tied or fewer than two players are active.
 */
int StageDataHolder::tryCalcLastStageBestScoreUserID() const {
    int bestUserId = -1;
    int bestScore = -1;
    int activeCount = 0;
    for (int i = 0; i < rc::getControlUserNumMax(); ++i) {
        if (!rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
            continue;
        }

        const int score = mpUsers[i].mScore;
        ++activeCount;
        if (bestScore < score) {
            bestUserId = i;
            bestScore = score;
        } else if (bestScore == score) {
            bestUserId = -1;
        }
    }

    if (activeCount <= 1) {
        return -1;
    }

    return bestUserId;
}

/**
 * @brief Update a player's stage state.
 * @param userId Control-user index within the allocated stage-user array.
 * @param alive New value to store in the player record.
 */
void StageDataHolder::setAlive(int userId, bool alive) { mpUsers[userId].setAlive(alive); }

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
bool StageDataHolder::isAlive(int userId) const { return mpUsers[userId].mAlive; }

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
 * @brief Update a player's stage state.
 * @param userId Control-user index within the allocated stage-user array.
 * @param figureType New value to store in the player record.
 */
void StageDataHolder::setPlayerFigureType(int userId, int figureType) {
    mpUsers[userId].setFigureType(figureType);
}

/**
 * @brief Read a player's stage result.
 * @param userId Control-user index within the allocated stage-user array.
 * @return The requested value from the player record.
 */
int StageDataHolder::getPlayerFigureType(int userId) const { return mpUsers[userId].mFigureType; }

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
 * @brief Restore the stock items saved in the playing file.
 */
void StageDataHolder::resetStockItems() {
    mpStockItems->copy(mpHolder->getPlayingFile()->getStockItemList());
}

/**
 * @brief Collect a green star, also keeping it for checkpoint restarts outside Captain Toad stages.
 * @param starIndex Zero-based green-star index.
 */
void StageDataHolder::acquireGreenStar(int starIndex) {
    CourseGreenStarInfo* pStars = mpStarsCurrent;
    const u32 bit = 1 << starIndex;
    pStars->mFlags |= bit;
    StageDatabaseInfo* pInfo =
        GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), mCourseId);
    if (pInfo == nullptr || !pInfo->isKinopioBrigade()) {
        mpStarsCheckpoint->mFlags |= bit;
    }
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::acquireIllustItem() {
    mStampCurrent = true;
    mStampCheckpoint = true;
}

/**
 * @brief Remember which character collected the stamp.
 * @param characterType Character identifier of the player collecting the stamp.
 */
void StageDataHolder::setStampPickupCharType(int characterType) { mStampCharacter = characterType; }

/**
 * @brief Read the current stage state.
 * @return True when the corresponding stage state is set.
 */
bool StageDataHolder::isAcquireIllustItem() const { return mStampCurrent; }

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::setUseAssistBlock() {
    mAssistCurrent = true;
    mAssistCheckpoint = true;
}

/**
 * @brief Check whether a green star was collected in the current attempt.
 * @param starIndex Zero-based green-star index.
 * @return True when the star is collected.
 */
bool StageDataHolder::isAcquireGreenStar(int starIndex) const {
    return mpStarsCurrent->isAcquired(starIndex);
}

/**
 * @brief Access the current green-star flags.
 * @return The current stage green-star record.
 */
const CourseGreenStarInfo* StageDataHolder::getGreenStarAcquireFlag() const { return mpStarsCurrent; }

/**
 * @brief Save the current progress as the checkpoint state.
 * @param characterType Character identifier of the player passing the checkpoint.
 */
void StageDataHolder::setCheckpointPass(int characterType) {
    mCheckpointCharacter = characterType;
    mpStarsCheckpoint->copy(*mpStarsCurrent);
    mStampCheckpoint = mStampCurrent;
    mAssistCheckpoint = mAssistCurrent;
}

/**
 * @brief Update stage progression state.
 */
void StageDataHolder::resetCheckpointPass() { mCheckpointCharacter = -1; }

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
    const int capped = count < 999 ? count : 999;
    return capped < 0 ? 0 : capped;
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

/**
 * @brief Build the world and stage database from the StageList BYAML.
 * @param pResource Resource containing the StageList BYAML.
 * @return A newly allocated world list.
 */
WorldInfoList* StageInfoFunction::createWorldInfoList(al::Resource* pResource) {
    al::ByamlIter root(pResource->getByml("StageList"));
    al::ByamlIter worldList;
    root.tryGetIterByKey(&worldList, "WorldList");
    const int worldNum = worldList.getSize();
    int stageNum = 0;
    for (int i = 0; i < worldNum; ++i) {
        al::ByamlIter world;
        al::ByamlIter stageList;
        worldList.tryGetIterByIndex(&world, i);
        world.tryGetIterByKey(&stageList, "StageList");
        stageNum += stageList.getSize();
    }

    WorldInfoList* pList = new WorldInfoList(worldNum, stageNum);
    int stageIndex = 0;
    for (int i = 0; i < worldNum; ++i) {
        al::ByamlIter world;
        worldList.tryGetIterByIndex(&world, i);
        if (!world.isValid()) {
            continue;
        }

        al::ByamlIter stageList;
        world.tryGetIterByKey(&stageList, "StageList");
        int worldId;
        world.tryGetIntByKey(&worldId, "WorldId");
        WorldInfo* pWorld = &pList->mpWorlds[i];
        pWorld->init(worldId, pList->getStageInfoByIndex(stageIndex), stageList.getSize());
        stageIndex += stageList.getSize();
        for (int j = 0; j < stageList.getSize(); ++j) {
            al::ByamlIter stage;
            stageList.tryGetIterByIndex(&stage, j);
            int stageId = 0;
            int courseId = 0;
            int greenStarNum = 0;
            int greenStarLock = 0;
            int stageTimer = 400;
            int illustItemNum = 0;
            int ghostId = -1;
            int ghostBaseTime = 0;
            int doubleMarioNum = 0;
            const char* pTypeName = "NormalStage";
            const char* pStageName = "";
            stage.tryGetIntByKey(&stageId, "StageId");
            stage.tryGetIntByKey(&courseId, "CourseId");
            stage.tryGetIntByKey(&greenStarNum, "GreenStarNum");
            stage.tryGetIntByKey(&greenStarLock, "GreenStarLock");
            stage.tryGetIntByKey(&stageTimer, "StageTimer");
            stage.tryGetIntByKey(&illustItemNum, "IllustItemNum");
            stage.tryGetIntByKey(&ghostId, "GhostId");
            stage.tryGetIntByKey(&ghostBaseTime, "GhostBaseTime");
            stage.tryGetIntByKey(&doubleMarioNum, "DoubleMarioNum");
            stage.tryGetStringByKey(&pStageName, "StageName");
            stage.tryGetStringByKey(&pTypeName, "StageType");
            pWorld->getStageInfoByIndex(j)->initialize(
                worldId, stageId, courseId, greenStarNum, greenStarLock, stageTimer, illustItemNum,
                ghostId, ghostBaseTime, doubleMarioNum, pTypeName, pStageName);
        }
    }

    return pList;
}
