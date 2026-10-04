#include "System/GameDataFile.hpp"
#include "Library/Math/MathUtil.hpp"
#include "System/ControlUserDataHolder.hpp"
#include "System/CourseInfoHolder.hpp"
#include "System/Data/SingleModeData.hpp"
#include "System/Data/StageDataHolder.hpp"
#include "System/Data/StageDatabaseInfo.hpp"
#include "System/Data/StageListHolder.hpp"
#include "System/Data/StockItemList.hpp"
#include "System/Data/WorldGameDataHolder.hpp"
#include "System/GameDataFlagFunction.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataHolderWriter.hpp"
#include "System/PlayLogFunction.hpp"
#include "System/SelectedGameDataFlagFunction.hpp"
#include "Util/ControlUserUtil.hpp"
#include <attributes.h>
#include <nn/oe.h>
#include <stream/seadStream.h>

namespace {

constexpr preport::KeyEventType cEventOptions = static_cast<preport::KeyEventType>(0);
constexpr int cEventIdOptions3DWorld = 6;
constexpr int cEventIdOptionsSingleMode = 10;

constexpr preport::Key cKeyCameraReverseHorizontal = static_cast<preport::Key>(3);
constexpr preport::Key cKeyCameraReverseVertical = static_cast<preport::Key>(4);
constexpr preport::Key cKeyCameraSensitivity = static_cast<preport::Key>(7);
constexpr preport::Key cKeyAssistModeType = static_cast<preport::Key>(8);
constexpr preport::Key cKeyFileId = static_cast<preport::Key>(9);
constexpr preport::Key cKeyFileName = static_cast<preport::Key>(10);
constexpr preport::Key cKeyPlayTime = static_cast<preport::Key>(11);
constexpr preport::Key cKeySinglePlayTime = static_cast<preport::Key>(12);
constexpr preport::Key cKeyGoalItemNum = static_cast<preport::Key>(34);
constexpr preport::Key cKeyIslandId = static_cast<preport::Key>(35);

constexpr int cPlayerLifePerPlayer = 5;
constexpr int cPlayerLifeMax = 1110;
constexpr int cRetryCountMax = 99999;
constexpr int cCasinoRoomCounterDefault = 5;
constexpr int cGoldenExpressCounterDefault = 20;
constexpr int cGhostPresentCounterMax = 19;
constexpr int cGhostPresentRandomRange = 21;
constexpr int cNormalWorldIdMax = 8;
constexpr int cSpecialWorldIdMax = 11;

/**
 * @brief Serialized layout of the 3D World part of a save file.
 */
struct GameDataFileSaveData {
    s32 mPlayerLife;
    s32 mReserved04;
    s32 mLastPlayCourseId;
    s32 mRetryCount;
    s32 mUnknown10;
    s32 mCasinoRoomCounter;
    u32 mGameFlag;
    s32 mGoldenExpressCounter;
    u8 mReserved20;
    u8 mMiiverseSetting;
    u8 mGhostSetting;
    u8 mReserved23;
    u16 mNetworkSetting;
    bool mUnknown26;
    bool mIsStartEnding;
    s32 mStockItems[4];
    s64 mUnknown38;
    s32 mMissCount;
    u8 mReserved44[0x204];
};
static_assert(sizeof(GameDataFileSaveData) == 0x248);

/**
 * @brief Map transient power-up forms to the form kept between stages.
 * @param figureType Figure type at the end of the stage.
 * @return The figure type to keep.
 */
int normalizeFigureType(int figureType) {
    if (figureType == 6) {
        figureType = 4;
    }

    if (figureType == 8) {
        figureType = 3;
    }

    if (figureType == 9) {
        figureType = 3;
    }

    return figureType;
}

/**
 * @brief Check whether every Green Star of the selected worlds has been collected.
 * @param accessor Accessor to an initialized game-data holder.
 * @param pCourses Course records to inspect.
 * @param worldIdMax Last world whose courses are counted.
 * @return True when all Green Stars of those worlds are collected.
 */
bool isCompleteGreenStar(GameDataHolderAccessor accessor, const CourseInfoHolder* pCourses,
                         int worldIdMax) {
    int courseNum = GameDataFunction::getCourseTotalNum(accessor);
    int acquireNum = 0;
    int totalNum = 0;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        int starNum = GameDataFunction::findCourseGreenStarNum(accessor, courseId);

        if (GameDataFunction::findStageDatabaseInfo(accessor, courseId)->getWorldId() <=
            worldIdMax) {
            acquireNum += pCourses->getCourseInfo(courseId)->calcGreenStarAcquireNum(starNum);
            totalNum += starNum;
        }
    }

    return acquireNum == totalNum;
}

/**
 * @brief Check whether every regular course up to the special worlds has been cleared.
 * @param accessor Accessor to an initialized game-data holder.
 * @param pCourses Course records to inspect.
 * @return True when every such course is cleared.
 */
bool isClearAllSpecialCourse(GameDataHolderAccessor accessor, const CourseInfoHolder* pCourses) {
    int courseNum = GameDataFunction::getCourseTotalNum(accessor);
    bool isClear = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        StageDatabaseInfo* pInfo = GameDataFunction::findStageDatabaseInfo(accessor, courseId);

        if (pInfo->getWorldId() <= cSpecialWorldIdMax && !pInfo->isEvent()) {
            isClear &= pCourses->getCourseInfo(courseId)->isClear();
        }
    }

    return isClear;
}

/**
 * @brief Check whether every regular course was cleared with every character.
 * @param accessor Accessor to an initialized game-data holder.
 * @param pCourses Course records to inspect.
 * @return True when every such course is completely cleared.
 */
bool isClearCompleteAllCourse(GameDataHolderAccessor accessor, const CourseInfoHolder* pCourses) {
    int courseNum = GameDataFunction::getCourseTotalNum(accessor);
    bool isComplete = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        StageDatabaseInfo* pInfo = GameDataFunction::findStageDatabaseInfo(accessor, courseId);

        if (pInfo->getWorldId() <= cSpecialWorldIdMax && !pInfo->isEvent()) {
            isComplete &=
                CourseInfoFunction::isClearCompleteCourseInfoHolder(pCourses, accessor, courseId);
        }
    }

    return isComplete;
}

/**
 * @brief Check whether every regular course was cleared by reaching the top of the goal pole.
 * @param accessor Accessor to an initialized game-data holder.
 * @param pCourses Course records to inspect.
 * @return True when every such course has the flag-top clear.
 */
bool isClearFlagTopAllCourse(GameDataHolderAccessor accessor, const CourseInfoHolder* pCourses) {
    int courseNum = GameDataFunction::getCourseTotalNum(accessor);
    bool isFlagTop = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        StageDatabaseInfo* pInfo = GameDataFunction::findStageDatabaseInfo(accessor, courseId);

        if (pInfo->getWorldId() <= cSpecialWorldIdMax && !pInfo->isEvent()) {
            isFlagTop &=
                CourseInfoFunction::isClearFlagTopCourseInfoHolder(pCourses, accessor, courseId);
        }
    }

    return isFlagTop;
}

/**
 * @brief Check whether every illustration item of the special worlds has been collected.
 * @param accessor Accessor to an initialized game-data holder.
 * @param pCourses Course records to inspect.
 * @return True when every course with an illustration item has it collected.
 */
bool isCompleteIllustItem(GameDataHolderAccessor accessor, const CourseInfoHolder* pCourses) {
    int courseNum = GameDataFunction::getCourseTotalNum(accessor);
    bool isComplete = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        if (GameDataFunction::findStageDatabaseInfo(accessor, courseId)->getWorldId() <=
                cSpecialWorldIdMax &&
            GameDataFunction::findIllustItemNum(accessor, courseId) >= 1) {
            isComplete &= CourseInfoFunction::isAcquireIllustItemCourseInfoHolder(
                pCourses, accessor, courseId);
        }
    }

    return isComplete;
}

} // namespace

u8 GameDataFile::sCameraSettings;

/**
 * @brief Read the packed camera settings.
 * @return The camera-option bits stored in the low byte.
 */
u16 GameDataFile::getOptions() { return sCameraSettings; }

/**
 * @brief Replace the packed camera settings.
 * @param options Camera-option bits; only the low eight bits are retained.
 */
void GameDataFile::setOptions(u16 options) { sCameraSettings = options; }

/**
 * @brief Construct an empty 3D World save file and allocate its records.
 * @param pHolder Game-data holder owning the file.
 * @param fileId Index of the save file.
 */
GameDataFile::GameDataFile(GameDataHolder* pHolder, int fileId)
    : GameDataFileBase(pHolder, fileId, false), mPlayerLife(cPlayerLifePerPlayer),
      mPlayerLifeSaved(cPlayerLifePerPlayer), mCasinoRoomCounter(cCasinoRoomCounterDefault),
      mGoldenExpressCounter(cGoldenExpressCounterDefault), mRetryCount(0), mRetryCountSaved(0),
      mUnknown98(0), mLastPlayCourseId(GameDataFunction::getInvalidCourseId()),
      mLastPlayCourseIdSaved(1), mTotalAcquireGreenStarNumSaved(0), mGameFlag(0),
      mUnknownAC(false), mIsStartEnding(false), mMiiverseSetting(1), mGhostSetting(1),
      mNetworkSetting(0), mpCourseInfoHolder(nullptr), mpWorldGameDataHolder(nullptr),
      mpStockItems(nullptr), mpStageDataHolder(nullptr), mLastTotalAcquireGreenStarNum(0),
      mLastStageBestScoreUserId(-1), mGhostPresentCounter(0), mClearStarLevel(0),
      mIsInsideSuperbView(false), mIsAlreadyShowBossDemo(false), mIsGameOver(false),
      mPlayWorldId(0), mMissCount(0), mPlayLogStageFlag(0), mStageStartActiveTime(),
      mStageStartTime(0) {
    mpStageDataHolder = new StageDataHolder(mpHolder);
    mpCourseInfoHolder = new CourseInfoHolder(mpHolder->getStageList()->getCourseTotalNum() + 1);
    mpWorldGameDataHolder = new WorldGameDataHolder();
    mpStockItems = new StockItemList(pHolder);
    initializeData();
}

/**
 * @brief Reset the file to the state of a new game.
 */
void GameDataFile::initializeData() {
    GameDataFileBase::initializeData(false);
    mPlayerLife = cPlayerLifePerPlayer;
    mPlayerLifeSaved = cPlayerLifePerPlayer;
    mCasinoRoomCounter = cCasinoRoomCounterDefault;
    mGoldenExpressCounter = cGoldenExpressCounterDefault;
    mRetryCount = 0;
    mRetryCountSaved = 0;
    mUnknown98 = 0;
    mLastPlayCourseId = 0;
    mLastPlayCourseIdSaved = 1;
    mTotalAcquireGreenStarNumSaved = 0;
    mGameFlag = 0;
    mUnknownAC = false;
    mIsStartEnding = false;
    mNetworkSetting = 0;
    mpStockItems->initialize();
    mMiiverseSetting = 1;
    mGhostSetting = 1;
    mMissCount = 0;
    mpStageDataHolder->initializeData();
    mpCourseInfoHolder->initialize();
    mpWorldGameDataHolder->initialize();
    mLastTotalAcquireGreenStarNum = 0;
    mLastStageBestScoreUserId = -1;
    mGhostPresentCounter = al::getRandom(cGhostPresentRandomRange);
    mClearStarLevel = 0;
    mIsInsideSuperbView = false;
    mIsAlreadyShowBossDemo = false;
    mIsGameOver = false;
    mPlayLogStageFlag = 0;
    mStageStartActiveTime = nn::oe::GetProgramTotalActiveTime();
    mUnknown110 = 0;
}

/**
 * @brief Check whether the file holds usable data.
 * @return True for a new file or a file with a last played course.
 */
bool GameDataFile::checkValid() {
    if (!mNewFile && mLastPlayCourseId == 0) {
        return false;
    }

    return true;
}

/**
 * @brief Replace this file's progress with a copy of another file.
 * @param rOther File to copy.
 */
void GameDataFile::copySaveFile(const GameDataFile& rOther) {
    initializeData();
    copyFileBase(rOther);
    mPlayerLife = rOther.mPlayerLife;
    mPlayerLifeSaved = rOther.mPlayerLifeSaved;
    mCasinoRoomCounter = rOther.mCasinoRoomCounter;
    mGoldenExpressCounter = rOther.mGoldenExpressCounter;
    mRetryCount = rOther.mRetryCount;
    mRetryCountSaved = rOther.mRetryCountSaved;
    mUnknown98 = rOther.mUnknown98;
    mLastPlayCourseId = rOther.mLastPlayCourseId;
    mLastPlayCourseIdSaved = rOther.mLastPlayCourseIdSaved;
    mTotalAcquireGreenStarNumSaved = rOther.mTotalAcquireGreenStarNumSaved;
    mUnknownAC = rOther.mUnknownAC;
    mIsStartEnding = rOther.mIsStartEnding;
    mNetworkSetting = rOther.mNetworkSetting;
    mGameFlag = rOther.mGameFlag;
    mMiiverseSetting = rOther.mMiiverseSetting;
    mGhostSetting = rOther.mGhostSetting;
    mMissCount = rOther.mMissCount;
    mpStageDataHolder->initializeData();
    mpStockItems->copy(rOther.mpStockItems);
    mpCourseInfoHolder->copy(rOther.mpCourseInfoHolder);
    mpWorldGameDataHolder->copy(rOther.mpWorldGameDataHolder);
}

/**
 * @brief Continue after a game over: refill lives and roll back the stage progress.
 */
void GameDataFile::recoverGameOver() {
    int playerNum = mpUsers->calcPlayablePlayerNum();
    mPlayerLife = (playerNum > 1 ? playerNum : 1) * cPlayerLifePerPlayer;
    int courseId = mpStageDataHolder->getCourseId();

    if (GameDataFunction::isStageContinuousMysteryBox(GameDataHolderAccessor(mpHolder),
                                                      courseId)) {
        CourseInfo* pCourse = mpCourseInfoHolder->getCourseInfo(courseId);
        pCourse->setGreenStarAcquireFlag(*mpStageDataHolder->getGreenStarAcquireFlag());
        PlayLogFunction::setGreenStarNum(GameDataHolderWriter(mpHolder), getPlayingCourseId(),
                                         pCourse->getGreenStarInfo().calcGreenStarAcquireNum(-1));
    }

    mpStockItems->initialize();
    mpUsers->recoverGameOver();
    mpStageDataHolder->gameOverStage();
    mLastStageBestScoreUserId = -1;
    GameDataFileInternal::resetKinopioHouse(this);
    int courseNum = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(mpHolder));
    int checkedWorldId = -1;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        StageDatabaseInfo* pInfo =
            GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), courseId);
        int worldId = pInfo->getWorldId();

        if (checkedWorldId != worldId) {
            bool isShown = GameDataFlagFunction::isShowWorldStartDemo(
                GameDataHolderAccessor(mpHolder), worldId);
            checkedWorldId = worldId;

            if (!isShown) {
                break;
            }
        }

        if (pInfo->isCasinoRoom()) {
            mpCourseInfoHolder->getCourseInfo(courseId)->openCasinoRoom();
        }
    }

    resetCasinoRoomCounter();
    mUnknownAC = true;
    mIsStartEnding = false;
    mIsGameOver = false;
}

/**
 * @brief Close every Toad House again and reset the world item flags.
 * @param pFile Save file to update.
 */
NOINLINE inline void GameDataFileInternal::resetKinopioHouse(GameDataFile* pFile) {
    for (int courseId = 1;
         courseId <= GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(pFile->mpHolder));
         courseId++) {
        if (GameDataFunction::isStageKinopioHouse(GameDataHolderAccessor(pFile->mpHolder),
                                                  courseId) ||
            GameDataFunction::isStageKinopioHouseHide(GameDataHolderAccessor(pFile->mpHolder),
                                                      courseId)) {
            pFile->mpCourseInfoHolder->getCourseInfo(courseId)->resetKinopioHouse();
        }
    }

    pFile->mpWorldGameDataHolder->resetAllItemFlag();
}

/**
 * @brief Continue after failing the golden express: count it as cleared and recover.
 */
void GameDataFile::recoverGameOverFromGoldenExpress() {
    mGoldenExpressCounter = cGoldenExpressCounterDefault;
    getCourseInfoPtr(mpStageDataHolder->getCourseId())->setClear();
    recoverGameOver();
    mpStageDataHolder->clearStage(true, false, false);
}

/**
 * @brief Apply this file's settings when it becomes the playing file.
 */
void GameDataFile::setPlayingFile() {
    PlayLogFunction::setMiiverseFlag(GameDataHolderWriter(mpHolder), mMiiverseSetting == 0);
}

/**
 * @brief Begin playing a stage.
 * @param worldId World of the stage.
 * @param stageId Stage within the world.
 */
void GameDataFile::startStage(int worldId, int stageId) {
    mPlayWorldId = worldId;
    int courseId =
        GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), worldId, stageId);
    mpStageDataHolder->startStage(courseId);
    mLastPlayCourseId = mpStageDataHolder->getCourseId();
    mLastTotalAcquireGreenStarNum = calcTotalAcquireGreenStarNum();
    mIsAlreadyShowBossDemo = false;
    mPlayLogStageFlag = 0;
    mGoalCharacterNum = 0;
    mGhostPresentCounter =
        mGhostPresentCounter > cGhostPresentCounterMax ? 0 : mGhostPresentCounter + 1;
    mStageStartActiveTime = nn::oe::GetProgramTotalActiveTime();
    mStageStartTime.setNow();

    if (worldId == 11 && stageId == 12) {
        mIsAlreadyShowBossDemo = true;
    }
}

/**
 * @brief Begin playing a stage in Bowser's Fury.
 * @param worldId World of the stage.
 * @param stageId Stage within the world.
 */
void GameDataFile::startSingleModeStage(int worldId, int stageId) {
    int courseId =
        GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), worldId, stageId);
    mpStageDataHolder->startStage(courseId);
    mPlayLogStageFlag = 0;
}

/**
 * @brief Record the start of the stage in the play log.
 */
void GameDataFile::onStageStart() {
    GameDataFileBase::onStageStart();
    mIsInsideSuperbView = false;

    if (!mpHolder->isSingleMode()) {
        updateTotalPlayTimePR();
        mUnknownF3 = false;
        mUnknownF4 = false;
    }

    if (mpStageDataHolder->isRestart()) {
        return;
    }

    if (!GameDataFunction::isStageKinopioBrigade(GameDataHolderAccessor(mpHolder),
                                                 getPlayingCourseId())) {
        for (int i = 0; i < rc::getControlUserNumMax(); i++) {
            if (rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
                PlayLogFunction::setPlayChara(
                    GameDataHolderWriter(mpHolder),
                    rc::getControlUserCharacterType(GameDataHolderAccessor(mpHolder), i));
            }
        }
    }

    PlayLogFunction::setUserNum(GameDataHolderWriter(mpHolder),
                                rc::getActiveControlUserNum(GameDataHolderAccessor(mpHolder)));
    PlayLogFunction::startStage(GameDataHolderWriter(mpHolder), getPlayingCourseId());

    for (int i = 0; i < rc::getControlUserNumMax(); i++) {
        if (!rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
            mpUsers->setPlayerFigureType(i, 0);
        }
    }
}

/**
 * @brief Record the end of the stage in the play log.
 */
void GameDataFile::onStageEnd() {
    GameDataFileBase::onStageEnd();
    mIsInsideSuperbView = false;
    PlayLogFunction::onStageEnd(GameDataHolderWriter(mpHolder));
}

/**
 * @brief Send the play report describing the selected options.
 */
void GameDataFile::playReportOptionsEvent() {
    preport::PlayReportManager* pManager = mpHolder->getPlayReportManager();

    if (pManager == nullptr) {
        return;
    }

    if (pManager->EventBegin(cEventOptions,
                             mpHolder->isSingleMode() ? cEventIdOptionsSingleMode :
                                                        cEventIdOptions3DWorld,
                             0, true)) {
        return;
    }

    GameDataHolder* pHolder = mpHolder;
    SingleModeData* pSingleFile = pHolder->getSingleFile();
    GameDataFile* pFile = pHolder->getGameDataFile(pHolder->getLastPlayingFileId());
    mpHolder->addSessionId();

    if (mpHolder->isSingleMode()) {
        mpHolder->getPlayReportManager()->KeySetValue(
            cKeyFileId, mpHolder->getLastSingleModePlayingFileID());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyFileName, pSingleFile->getName());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyPlayTime,
                                                      pFile->getTotalPlayTimePR(false));
        mpHolder->getPlayReportManager()->KeySetValue(cKeySinglePlayTime,
                                                      pSingleFile->getTotalPlayTimePR(false));
        const SingleModeData::Options& rOptions = SingleModeData::sOptions;
        mpHolder->getPlayReportManager()->KeySetValue(cKeyCameraReverseHorizontal,
                                                      rOptions.mIsCameraReverseHorizontal);
        mpHolder->getPlayReportManager()->KeySetValue(cKeyCameraReverseVertical,
                                                      rOptions.mIsCameraReverseVertical);
        mpHolder->getPlayReportManager()->KeySetValue(cKeyCameraSensitivity,
                                                      rOptions.mCameraSensitivity);
        mpHolder->getPlayReportManager()->KeySetValue(cKeyAssistModeType,
                                                      rOptions.mAssistModeType);
        mpHolder->getPlayReportManager()->KeySetValue(cKeyGoalItemNum,
                                                      pSingleFile->getGoalItemNum());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyIslandId,
                                                      pSingleFile->getCurValidIslandVisited());
    } else {
        mpHolder->getPlayReportManager()->KeySetValue(cKeyFileId,
                                                      mpHolder->getLastPlayingFileId());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyFileName, pSingleFile->getName());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyPlayTime,
                                                      pFile->getTotalPlayTimePR(false));
        mpHolder->getPlayReportManager()->KeySetValue(cKeySinglePlayTime,
                                                      pSingleFile->getTotalPlayTimePR(false));
        mpHolder->getPlayReportManager()->KeySetValue(cKeyCameraReverseHorizontal,
                                                      getCameraReverseHorizontal());
        mpHolder->getPlayReportManager()->KeySetValue(cKeyCameraReverseVertical,
                                                      getCameraReverseVertical());
    }

    mpHolder->getPlayReportManager()->EventEnd();
}

/**
 * @brief Check whether the camera axis is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getCameraReverseHorizontal() { return (sCameraSettings & 2) != 0; }

/**
 * @brief Check whether the camera axis is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getCameraReverseVertical() { return (sCameraSettings & 1) != 0; }

/**
 * @brief Send a stage play report; reports are not sent for 3D World stages.
 * @param eventId Identifier of the stage event.
 */
void GameDataFile::playReportStageEvent(int eventId) {}

/**
 * @brief Make the first active player the main player before saving in the course select.
 */
void GameDataFile::onSaveStartInCourseSelect() {
    mMainUserId = rc::getActiveControlUserFirst(GameDataHolderAccessor(mpHolder));
}

/**
 * @brief Handle a world warp; nothing has to be updated.
 * @param worldId Destination world.
 */
void GameDataFile::onWorldWarp(int worldId) {}

/**
 * @brief Move the last played course to the first stage of the next world.
 */
void GameDataFile::onWorldWarpDokanDemo() {
    int worldId;
    int stageId;
    GameDataFunction::calcWorldAndStageId(GameDataHolderAccessor(mpHolder), &worldId, &stageId,
                                          mLastPlayCourseId);
    mLastPlayCourseId =
        GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), worldId + 1, 1);
}

/**
 * @brief Move the last played course into the world shown by the title screen.
 * @param worldId World the title screen returns to.
 */
void GameDataFile::onGotoTitle(int worldId) {
    int lastWorldId;
    int stageId;
    GameDataFunction::calcWorldAndStageId(GameDataHolderAccessor(mpHolder), &lastWorldId,
                                          &stageId, mLastPlayCourseId);

    if (lastWorldId != worldId) {
        mLastPlayCourseId =
            GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), worldId, 1);
    }
}

/**
 * @brief Remember the player who set the best score in the stage.
 */
void GameDataFile::updateBestScoreUser() {
    mLastStageBestScoreUserId = mpStageDataHolder->tryCalcLastStageBestScoreUserID();
}

/**
 * @brief Remember the player who set the best score in the stage.
 * @param userId Control-user index, or -1 for none.
 */
void GameDataFile::setBestScoreUserId(int userId) { mLastStageBestScoreUserId = userId; }

/**
 * @brief Commit the result of a cleared stage to the course records.
 */
void GameDataFile::clearStage() {
    mMainUserId = rc::getActiveControlUserFirst(GameDataHolderAccessor(mpHolder));
    int courseId = mpStageDataHolder->getCourseId();
    CourseInfo* pCourse = mpCourseInfoHolder->getCourseInfo(courseId);

    if (!GameDataFunction::isStageKinopioBrigade(GameDataHolderAccessor(mpHolder), courseId)) {
        mGoalCharacterNum = 0;
        int aliveNum = 0;

        for (int i = 0; i < rc::getControlUserNumMax(); i++) {
            if (!rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
                continue;
            }

            if (getPlayerLife() != 0 || mpStageDataHolder->isAlive(i)) {
                aliveNum++;
            } else {
                mpUsers->getControlUserDataPtr(i)->setDeadInStage();
            }

            int figureType = normalizeFigureType(mpStageDataHolder->getPlayerFigureType(i));
            mpUsers->setPlayerFigureType(i, figureType);

            if (mpStageDataHolder->isGoalSuccess(i)) {
                int characterType =
                    rc::getControlUserCharacterType(GameDataHolderAccessor(mpHolder), i);
                f32 goalHeight = mpStageDataHolder->getGoalHeight(i);

                if (mpStageDataHolder->isGoalLeader(i)) {
                    pCourse->setClearInfo(characterType, goalHeight);
                }

                if (goalHeight == 1.0f) {
                    mGoalCharacters[mGoalCharacterNum++] = characterType;
                }

                pCourse->setClearCharacter(characterType);
            }
        }

        if (aliveNum == 0) {
            for (int i = 0; i < rc::getControlUserNumMax(); i++) {
                if (mpUsers->getControlUserDataPtr(i)->isDeadInStage()) {
                    mpUsers->getControlUserDataPtr(i)->resetDeadInStage();
                }
            }
        }
    }

    int time = mpStageDataHolder->isRestartFromCheckpoint() ?
                   -1 :
                   mpStageDataHolder->calcTimeAttackCount();
    int score = mpStageDataHolder->getTotalScore();

    if (GameDataFunction::isStageNormal(GameDataHolderAccessor(mpHolder), courseId) ||
        GameDataFunction::isStageKoopaCastle(GameDataHolderAccessor(mpHolder), courseId) ||
        GameDataFunction::isStageGateKeeper(GameDataHolderAccessor(mpHolder), courseId) ||
        GameDataFunction::isStageContinuousMysteryBox(GameDataHolderAccessor(mpHolder),
                                                      courseId)) {
        if (mpStageDataHolder->isUseAssistBlock()) {
            pCourse->setClearWithAssistBlock();
            score = 0;
            time = -1;
        } else {
            pCourse->resetClearWithAssistBlock();
        }
    }

    const CourseGreenStarInfo* pStars = mpStageDataHolder->getGreenStarAcquireFlag();
    bool isAcquireIllustItem = mpStageDataHolder->isAcquireIllustItem();
    CourseInfo* pPlayingCourse = getCourseInfoPtr(courseId);
    pPlayingCourse->setOpen();
    pPlayingCourse->setGreenStarAcquireFlag(*pStars);
    pPlayingCourse->setAcquireIllustItem(isAcquireIllustItem);
    bool isFirstClear = pPlayingCourse->setClear();
    bool isNewBestScore = pPlayingCourse->setBestScore(score);
    bool isNewBestTime = pPlayingCourse->setBestTime(time);

    if (GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), courseId)
            ->isNormal()) {
        int casinoRoomCounter = mCasinoRoomCounter - 1;
        mCasinoRoomCounter = casinoRoomCounter < 0 ? 0 : casinoRoomCounter;
        int goldenExpressCounter = mGoldenExpressCounter - 1;
        mGoldenExpressCounter = goldenExpressCounter < 0 ? 0 : goldenExpressCounter;
    }

    if (GameDataFunction::isStageNormal(GameDataHolderAccessor(mpHolder), courseId) ||
        GameDataFunction::isStageKoopaCastle(GameDataHolderAccessor(mpHolder), courseId)) {
        mpWorldGameDataHolder->resetItemFlag();
    }

    if (GameDataFunction::isStageGoldenExpress(GameDataHolderAccessor(mpHolder), courseId)) {
        mGoldenExpressCounter = cGoldenExpressCounterDefault;
    }

    PlayLogFunction::clearStage(
        GameDataHolderWriter(mpHolder), getPlayingCourseId(),
        mpStageDataHolder->calcTimeAttackCount(),
        mpStageDataHolder->getGreenStarAcquireFlag()->calcGreenStarAcquireNum(-1),
        mpStageDataHolder->isAcquireIllustItem(), mpStageDataHolder->isUseAssistBlock());
    mpStageDataHolder->clearStage(isFirstClear, isNewBestScore, isNewBestTime);
    mpStockItems->copy(mpStageDataHolder->getStockItemList());
    mLastPlayCourseId = mpStageDataHolder->getCourseId();
    calcClearStarLevel();
}

/**
 * @brief Commit the result of a stage left through a world warp.
 */
void GameDataFile::clearStageWorldWarp() {
    int courseId = mpStageDataHolder->getCourseId();

    for (int i = 0; i < rc::getControlUserNumMax(); i++) {
        if (getPlayerLife() == 0 && rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i) &&
            !mpStageDataHolder->isAlive(i)) {
            mpUsers->getControlUserDataPtr(i)->setDeadInStage();
        }

        if (rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
            int figureType = normalizeFigureType(mpStageDataHolder->getPlayerFigureType(i));
            mpUsers->setPlayerFigureType(i, figureType);
        }
    }

    CourseInfo* pCourse = mpCourseInfoHolder->getCourseInfo(courseId);
    pCourse->setGreenStarAcquireFlag(*mpStageDataHolder->getGreenStarAcquireFlag());
    pCourse->setAcquireIllustItem(mpStageDataHolder->isAcquireIllustItem());
    pCourse->setWorldWarpClear();
    mpStageDataHolder->clearStageWorldWarp();
    mpStockItems->copy(mpStageDataHolder->getStockItemList());
}

/**
 * @brief Keep the Green Stars collected in the stage or report them to the play log.
 * @param isPlayLog True to only report the stars of a continuous mystery box.
 */
void GameDataFile::trySaveCourseGreenStars(bool isPlayLog) {
    int courseId = mpStageDataHolder->getCourseId();
    CourseInfo* pCourse = mpCourseInfoHolder->getCourseInfo(courseId);

    if (GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), courseId)
            ->isKinopioBrigade()) {
        return;
    }

    if (isPlayLog) {
        if (GameDataFunction::isStageContinuousMysteryBox(GameDataHolderAccessor(mpHolder),
                                                          courseId)) {
            PlayLogFunction::setGreenStarNum(
                GameDataHolderWriter(mpHolder), getPlayingCourseId(),
                pCourse->getGreenStarInfo().calcGreenStarAcquireNum(-1));
        }
    } else {
        pCourse->setGreenStarAcquireFlag(*mpStageDataHolder->getGreenStarAcquireFlag());
    }
}

/**
 * @brief Leave the stage without clearing it.
 */
void GameDataFile::retireStage() {
    trySaveCourseGreenStars(true);
    mpStageDataHolder->retireStage();
    mLastPlayCourseId = mpStageDataHolder->getCourseId();
}

/**
 * @brief Leave the stage through its exit door, keeping the players' forms.
 */
void GameDataFile::retireStageExitDoor() {
    for (int i = 0; i < rc::getControlUserNumMax(); i++) {
        if (rc::isActiveControlUser(GameDataHolderAccessor(mpHolder), i)) {
            int figureType = normalizeFigureType(mpStageDataHolder->getPlayerFigureType(i));
            mpUsers->setPlayerFigureType(i, figureType);
        }
    }

    mpStageDataHolder->retireStage();
    mpStockItems->copy(mpStageDataHolder->getStockItemList());
    mLastPlayCourseId = mpStageDataHolder->getCourseId();
}

/**
 * @brief Restart the stage from the beginning.
 */
void GameDataFile::restartStage() {
    mpUsers->resetFigureType();
    mpStageDataHolder->restartStage();
    mpStockItems->copy(mpStageDataHolder->getStockItemList());
    mLastStageBestScoreUserId = -1;
    mGoalCharacterNum = 0;
    mGhostPresentCounter =
        mGhostPresentCounter > cGhostPresentCounterMax ? 0 : mGhostPresentCounter + 1;
}

/**
 * @brief Forget the scores of the stage.
 */
void GameDataFile::resetStageScore() {
    mLastStageBestScoreUserId = -1;
    mpStageDataHolder->resetStageScore();
}

/**
 * @brief Restart a Captain Toad stage.
 */
void GameDataFile::restartKinopioBrigade() { mpStageDataHolder->restartStage(); }

/**
 * @brief Restart a mystery box, keeping the Green Stars collected so far.
 */
void GameDataFile::restartMysteryBox() {
    trySaveCourseGreenStars(false);
    mLastStageBestScoreUserId = -1;
    mpStageDataHolder->restartMysteryBox();
    mpUsers->resetFigureType();
    mpStockItems->copy(mpStageDataHolder->getStockItemList());
}

/**
 * @brief Restart a mystery box after the timer ran out.
 */
void GameDataFile::restartTimeupMysteryBox() {
    trySaveCourseGreenStars(false);
    mpStageDataHolder->restartTimeupMysteryBox();
}

/**
 * @brief Enter the stage again.
 */
void GameDataFile::reenterStage() { mpStageDataHolder->reenterStage(); }

/**
 * @brief Move the last played course to the start of a world for its opening demo.
 * @param worldId World whose start demo is played.
 */
void GameDataFile::playWorldStartDemo(int worldId) {
    mLastPlayCourseId =
        GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), worldId, 1);
}

/**
 * @brief Start a new game and give every player their lives.
 */
void GameDataFile::startOpening() {
    GameDataFileBase::startOpening();
    int playerNum = mpUsers->calcPlayablePlayerNum();
    mPlayerLife = (playerNum > 1 ? playerNum : 1) * cPlayerLifePerPlayer;
    mIsGameOver = false;
}

/**
 * @brief Return to the first course after the ending.
 */
void GameDataFile::startEnding() {
    mLastPlayCourseId = GameDataFunction::calcCourseId(GameDataHolderAccessor(mpHolder), 1, 1);
    mIsStartEnding = true;
    GameDataFileInternal::resetKinopioHouse(this);
}

/**
 * @brief Restart the countdown to the next casino room.
 */
void GameDataFile::resetCasinoRoomCounter() { mCasinoRoomCounter = cCasinoRoomCounterDefault; }

/**
 * @brief Access the record of a course.
 * @param courseId Course identifier.
 * @return The course record.
 */
const CourseInfo* GameDataFile::getCourseInfo(int courseId) const {
    return mpCourseInfoHolder->getCourseInfo(courseId);
}

/**
 * @brief Access the record of a course.
 * @param courseId Course identifier.
 * @return The course record.
 */
CourseInfo* GameDataFile::getCourseInfoPtr(int courseId) {
    return mpCourseInfoHolder->getCourseInfo(courseId);
}

/**
 * @brief Access the record of a world.
 * @param worldId World identifier.
 * @return The world record.
 */
WorldGameData* GameDataFile::getWorldGameData(int worldId) const {
    return mpWorldGameDataHolder->getWorldGameData(worldId);
}

/**
 * @brief Enter a player with the requested character.
 * @param userId Control-user index from 0 through 3.
 * @param characterType Requested character; -1 preserves the current selection.
 * @return True when the character can be assigned.
 */
bool GameDataFile::entryPlayer(int userId, int characterType) {
    if (mStageStarted) {
        PlayLogFunction::setPlayerEntry(GameDataHolderWriter(mpHolder));
    }

    return GameDataFileBase::entryPlayer(userId, characterType);
}

/**
 * @brief Set the number of lives.
 * @param life New number of lives.
 */
void GameDataFile::initPlayerLife(int life) {
    mPlayerLife = life;
    mIsGameOver = false;
}

/**
 * @brief Read the number of lives.
 * @return The number of lives.
 */
int GameDataFile::getPlayerLife() const { return mPlayerLife; }

/**
 * @brief Read the number of lives at the last save.
 * @return The saved number of lives.
 */
int GameDataFile::getPlayerLifeSaved() const { return mPlayerLifeSaved; }

/**
 * @brief Add or remove lives; losing a life counts as a miss.
 * @param life Number of lives to add; negative to remove lives.
 * @return The resulting number of lives.
 */
int GameDataFile::addPlayerLife(int life) {
    if (life < 0) {
        GameDataFunction::addMissCount(GameDataHolderWriter(mpHolder));
        int retryCount = mRetryCount + 1;
        mRetryCount = retryCount < cRetryCountMax ? retryCount : cRetryCountMax;
        mMissCount++;
    }

    mPlayerLife += life;

    if (mPlayerLife < 0) {
        mIsGameOver = true;
        mPlayerLife = 0;
    } else {
        mIsGameOver = false;

        if (mPlayerLife > cPlayerLifeMax) {
            mPlayerLife = cPlayerLifeMax;
        }
    }

    return mPlayerLife;
}

/**
 * @brief Check whether all lives were lost.
 * @return True after a game over.
 */
bool GameDataFile::isGameOver() const { return mIsGameOver; }

/**
 * @brief Read the course being played.
 * @return The playing course identifier.
 */
int GameDataFile::getPlayingCourseId() const { return mpStageDataHolder->getCourseId(); }

/**
 * @brief Read the course played last.
 * @return The last played course identifier.
 */
int GameDataFile::getLastPlayCourseId() const { return mLastPlayCourseId; }

/**
 * @brief Check whether the last played course was cleared.
 * @return True when it was cleared.
 */
bool GameDataFile::isLastPlayCourseClear() const { return mpStageDataHolder->isCleared(); }

/**
 * @brief Check whether the last played course was left through a world warp.
 * @return True when it was cleared by a warp.
 */
bool GameDataFile::isLastPlayCourseClearWorldWarp() const {
    return mpStageDataHolder->isWorldWarpClear();
}

/**
 * @brief Check whether the last played course was cleared for the first time.
 * @return True for a first clear.
 */
bool GameDataFile::isLastPlayCourseFirstClear() const { return mpStageDataHolder->isFirstClear(); }

/**
 * @brief Check whether the illustration item of the last played course was newly collected.
 * @return True when it was newly collected.
 */
bool GameDataFile::isLastPlayCourseFirstAcquireIllustItem() const {
    return mpStageDataHolder->isFirstStamp();
}

/**
 * @brief Check whether the last played course set a new best score.
 * @return True for a new best score.
 */
bool GameDataFile::isLastPlayCourseUpdateBestScore() const {
    return mpStageDataHolder->isNewBestScore();
}

/**
 * @brief Check whether the last played course set a new best time.
 * @return True for a new best time.
 */
bool GameDataFile::isLastPlayCourseUpdateBestTime() const {
    return mpStageDataHolder->isNewBestTime();
}

/**
 * @brief Check whether clearing the last course opens a casino room.
 * @return True when a casino room has to be opened.
 */
bool GameDataFile::isNeedOpenCasinoRoom() const {
    if (!isLastPlayCourseClear()) {
        return false;
    }

    if (!GameDataFunction::isStageNormal(GameDataHolderAccessor(mpHolder),
                                         getLastPlayCourseId())) {
        return false;
    }

    return mCasinoRoomCounter == 0;
}

/**
 * @brief Check whether clearing the last course opens the golden express.
 * @return True when the golden express has to be opened.
 */
bool GameDataFile::isNeedOpenGoldenExpress() const {
    if (!isLastPlayCourseClear()) {
        return false;
    }

    return mGoldenExpressCounter == 0;
}

/**
 * @brief Check whether the progress has to be saved after the stage.
 * @return True unless the stage was retired.
 */
bool GameDataFile::isNeedSave() const { return !mpStageDataHolder->isRetired(); }

/**
 * @brief Check whether the final Bowser course has been cleared.
 * @return True after the normal ending.
 */
bool GameDataFile::isClearNormalEnding() const {
    int courseId = GameDataFunction::getLastKoopaCourseId(GameDataHolderAccessor(mpHolder));
    return mpCourseInfoHolder->getCourseInfo(courseId)->isClear();
}

/**
 * @brief Save the current progress values as the saved ones.
 */
void GameDataFile::onSave() {
    mpUsers->onSave();
    mPlayerLifeSaved = mPlayerLife;
    mRetryCountSaved = mRetryCount;
    mLastPlayCourseIdSaved = mLastPlayCourseId;

    if (mpHolder->isSingleMode()) {
        return;
    }

    mTotalAcquireGreenStarNumSaved = calcTotalAcquireGreenStarNum();
}

/**
 * @brief Change the Miiverse setting.
 * @param setting New Miiverse setting; zero enables posting.
 */
void GameDataFile::setMiiverseSetting(int setting) {
    mMiiverseSetting = setting;
    PlayLogFunction::setMiiverseFlag(GameDataHolderWriter(mpHolder), setting == 0);
}

/**
 * @brief Change the ghost setting.
 * @param setting New ghost setting.
 */
void GameDataFile::setGhostSetting(int setting) { mGhostSetting = setting; }

/**
 * @brief Count the Green Stars collected in every course.
 * @return The number of collected Green Stars.
 */
int GameDataFile::calcTotalAcquireGreenStarNum() const {
    int courseNum = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(mpHolder));
    int acquireNum = 0;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        if (GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), courseId)
                ->isKinopioBrigade() &&
            !mpCourseInfoHolder->getCourseInfo(courseId)->isClear()) {
            continue;
        }

        if (mpCourseInfoHolder->getCourseInfo(courseId)->isClear() ||
            mpCourseInfoHolder->getCourseInfo(courseId)->isWorldWarpClear() ||
            GameDataFunction::isStageContinuousMysteryBox(GameDataHolderAccessor(mpHolder),
                                                          courseId)) {
            int starNum = GameDataFunction::findCourseGreenStarNum(
                GameDataHolderAccessor(mpHolder), courseId);
            acquireNum +=
                mpCourseInfoHolder->getCourseInfo(courseId)->calcGreenStarAcquireNum(starNum);
        }
    }

    return acquireNum;
}

/**
 * @brief Count the collected illustration items and all-clear character stamps.
 * @return The number of collected items.
 */
int GameDataFile::calcTotalIllustItemNum() const {
    int courseNum = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(mpHolder));
    int itemNum = 0;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        if (mpCourseInfoHolder->getCourseInfo(courseId)->isClear() ||
            mpCourseInfoHolder->getCourseInfo(courseId)->isWorldWarpClear()) {
            itemNum += mpCourseInfoHolder->getCourseInfo(courseId)->isAcquireIllustItem();
        }
    }

    for (int characterType = 0; characterType < 5; characterType++) {
        itemNum +=
            SelectedGameDataFlagFunction::isAlreadyOpenAllClearCharacter(this, characterType);
    }

    return itemNum;
}

/**
 * @brief Compute how many stars the file has earned.
 * @return The clear star level from 0 through 5.
 */
int GameDataFile::calcClearStarLevel() {
    int level;

    if (!isClearNormalEnding()) {
        level = 0;
    } else if (!isCompleteGreenStar(GameDataHolderAccessor(mpHolder), getCourseInfoHolder(),
                                    cNormalWorldIdMax)) {
        level = 1;
    } else if (!isClearAllSpecialCourse(GameDataHolderAccessor(mpHolder),
                                        getCourseInfoHolder())) {
        level = 2;
    } else if (!isCompleteGreenStar(GameDataHolderAccessor(mpHolder), getCourseInfoHolder(),
                                    cSpecialWorldIdMax)) {
        level = 3;
    } else if (!isClearCompleteAllCourse(GameDataHolderAccessor(mpHolder),
                                         getCourseInfoHolder()) ||
               !isCompleteIllustItem(GameDataHolderAccessor(mpHolder), getCourseInfoHolder())) {
        level = 4;
    } else {
        level = 5;
    }

    mClearStarLevel = level;
    return level;
}

/**
 * @brief Check whether every course was cleared without the assist block.
 * @return True when no course was cleared with the assist block.
 */
bool GameDataFile::isTwinkleData() const {
    int courseNum = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(mpHolder));
    bool isTwinkle = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        isTwinkle &= !mpCourseInfoHolder->getCourseInfo(courseId)->isClearWithAssistBlock();
    }

    return isTwinkle;
}

/**
 * @brief Check whether every course was cleared with a character.
 * @param characterType Playable character identifier.
 * @return True when every regular course was cleared with the character.
 */
bool GameDataFile::isAllClearWithCharacter(int characterType) const {
    int courseNum = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(mpHolder));
    bool isAllClear = true;

    for (int courseId = 1; courseId <= courseNum; courseId++) {
        StageDatabaseInfo* pInfo =
            GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(mpHolder), courseId);

        if (!pInfo->isEvent() && !pInfo->isKinopioBrigade()) {
            isAllClear &= CourseInfoFunction::isClearCharacter(GameDataHolderAccessor(mpHolder),
                                                               courseId, characterType);
        }
    }

    return isAllClear;
}

/**
 * @brief Check whether Rosetta became playable by clearing the last course.
 * @return True when the course that unlocks Rosetta was just cleared.
 */
bool GameDataFile::isRosettaPlayable() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcRosettaAppearanceCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether best times are shown.
 * @return True once the file has a clear star.
 */
bool GameDataFile::isShowBestTime() const { return getClearStarLevel() > 0; }

/**
 * @brief Check whether World Star is open.
 * @return True once the file has a clear star.
 */
bool GameDataFile::isOpenWorldStar() const { return getClearStarLevel() > 0; }

/**
 * @brief Check whether clearing the last course opened the arrange world.
 * @return True when the course that opens it was just cleared.
 */
bool GameDataFile::isOpenWorldArrange() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcOpenWorldArrangeCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the Champion's Road world is open.
 * @return True when the special worlds are fully completed.
 */
bool GameDataFile::isOpenWorldChampionship() const {
    if (isCompleteGreenStar(GameDataHolderAccessor(mpHolder), mpCourseInfoHolder,
                            cSpecialWorldIdMax) &&
        isClearFlagTopAllCourse(GameDataHolderAccessor(mpHolder), mpCourseInfoHolder) &&
        isCompleteIllustItem(GameDataHolderAccessor(mpHolder), mpCourseInfoHolder)) {
        return true;
    }

    return false;
}

/**
 * @brief Check whether the world-jump menu explanation is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowWorldJumpMenuInfo() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcShowWorldJumpMenuInfoCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the second world-jump menu explanation is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowWorldJumpMenuInfo2nd() const {
    if (getLastPlayCourseId() != GameDataFunction::calcShowWorldJumpMenuInfo2ndCourseId(
                                     GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the network guide is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowNetworkGuide() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcShowNetworkGuideCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the amiibo guide is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowAmiiboGuide() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcShowAmiiboGuideCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the snapshot guide is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowSnapshotGuide() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcShowSnapshotGuideCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the touch guide is shown.
 * @return True while playing the course that shows it.
 */
bool GameDataFile::isShowTouchGuide() const {
    return getPlayingCourseId() ==
           GameDataFunction::calcShowTouchGuideCourseId(GameDataHolderAccessor(mpHolder));
}

/**
 * @brief Check whether the last cleared course was a Captain Toad course.
 * @return True when a Captain Toad course was just cleared.
 */
bool GameDataFile::isLastClearStageKinopioBrigade() const {
    if (GameDataFunction::isInvalidCourseId(getLastPlayCourseId())) {
        return false;
    }

    if (!GameDataFunction::isStageKinopioBrigade(GameDataHolderAccessor(mpHolder),
                                                 getLastPlayCourseId())) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the last cleared course used the touch screen.
 * @return True when a touch-screen course other than a Captain Toad course was just cleared.
 */
bool GameDataFile::isLastClearStageUseDrc() const {
    if (GameDataFunction::isInvalidCourseId(getLastPlayCourseId())) {
        return false;
    }

    if (!GameDataFunction::isStageUseDrc(GameDataHolderAccessor(mpHolder),
                                         getLastPlayCourseId())) {
        return false;
    }

    if (GameDataFunction::isStageKinopioBrigade(GameDataHolderAccessor(mpHolder),
                                                getLastPlayCourseId())) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the character-change explanation is shown.
 * @return True when the course that triggers it was just cleared.
 */
bool GameDataFile::isShowCharacterChangeExplain() const {
    if (getLastPlayCourseId() != GameDataFunction::calcShowCharacterChangeExplainCourseId(
                                     GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether clearing the last course opened the network settings.
 * @return True when the course that opens them was just cleared.
 */
bool GameDataFile::isOpenNetworkSetting() const {
    if (getLastPlayCourseId() !=
        GameDataFunction::calcOpenNetworkSettingCourseId(GameDataHolderAccessor(mpHolder))) {
        return false;
    }

    return isLastPlayCourseClear();
}

/**
 * @brief Check whether the boss demo can be skipped.
 * @return True when the demo was already shown or the course was cleared before.
 */
bool GameDataFile::isEnableCancelBossDemo() const {
    if (mIsAlreadyShowBossDemo) {
        return true;
    }

    return CourseInfoFunction::isClear(GameDataHolderAccessor(mpHolder), getPlayingCourseId());
}

/**
 * @brief Restore the default camera direction settings.
 */
void GameDataFile::initCameraSettings() { sCameraSettings = 0; }

/**
 * @brief Change the camera axis reversal option.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setCameraReverseVertical(GameDataHolder* pHolder, bool reverse) {
    if (reverse) {
        sCameraSettings |= 1;
    } else {
        sCameraSettings &= ~1;
    }
}

/**
 * @brief Change the camera axis reversal option.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setCameraReverseHorizontal(GameDataHolder* pHolder, bool reverse) {
    if (reverse) {
        sCameraSettings |= 2;
    } else {
        sCameraSettings &= ~2;
    }
}

/**
 * @brief Change the camera axis reversal option of Captain Toad courses.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setKinopioBrigadeCameraReverseVertical(GameDataHolder* pHolder, bool reverse) {
    if (reverse) {
        sCameraSettings |= 4;
    } else {
        sCameraSettings &= ~4;
    }
}

/**
 * @brief Check whether the camera axis of Captain Toad courses is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getKinopioBrigadeCameraReverseVertical() { return (sCameraSettings & 4) != 0; }

/**
 * @brief Change the camera axis reversal option of Captain Toad courses.
 * @param pHolder Game-data holder; unused because camera settings are shared.
 * @param reverse Whether to reverse this camera axis.
 */
void GameDataFile::setKinopioBrigadeCameraReverseHorizontal(GameDataHolder* pHolder,
                                                            bool reverse) {
    if (reverse) {
        sCameraSettings |= 8;
    } else {
        sCameraSettings &= ~8;
    }
}

/**
 * @brief Check whether the camera axis of Captain Toad courses is reversed.
 * @return True when the axis reversal option is enabled.
 */
bool GameDataFile::getKinopioBrigadeCameraReverseHorizontal() {
    return (sCameraSettings & 8) != 0;
}

/**
 * @brief Load the file from a save stream.
 * @param pStream Stream positioned at the file data.
 * @return True when the file was read successfully.
 */
bool GameDataFile::readFromStream(sead::ReadStream* pStream) {
    if (!read3DWorldFromStream(pStream)) {
        return false;
    }

    calcClearStarLevel();
    return true;
}

/**
 * @brief Load the 3D World data from a save stream.
 * @param pStream Stream positioned at the file data.
 * @return True when the file was read successfully.
 */
bool GameDataFile::read3DWorldFromStream(sead::ReadStream* pStream) {
    if (!GameDataFileBase::readFromStream(pStream)) {
        return false;
    }

    s32 size;
    pStream->readS32(size);
    GameDataFileSaveData data = {};
    pStream->readMemBlock(&data, sizeof(data));
    mPlayerLife = data.mPlayerLife;
    mLastPlayCourseId = data.mLastPlayCourseId;
    mRetryCount = data.mRetryCount;
    mUnknown98 = data.mUnknown10;
    mCasinoRoomCounter = data.mCasinoRoomCounter;
    mGoldenExpressCounter = data.mGoldenExpressCounter;
    mGameFlag = data.mGameFlag;
    mMiiverseSetting = data.mMiiverseSetting;
    mGhostSetting = data.mGhostSetting;
    mNetworkSetting = data.mNetworkSetting;
    mUnknownAC = data.mUnknown26;
    mIsStartEnding = data.mIsStartEnding;
    mUnknown110 = data.mUnknown38;
    mMissCount = data.mMissCount;

    for (int i = 0; i < 4; i++) {
        mpStockItems->setStockItem(i, data.mStockItems[i]);
    }

    if (!mpCourseInfoHolder->readFromStream(pStream)) {
        return false;
    }

    if (!mpWorldGameDataHolder->readFromStream(pStream)) {
        return false;
    }

    calcClearStarLevel();
    onSave();
    return true;
}

/**
 * @brief Write the file to a save stream.
 * @param pStream Destination stream.
 * @param isSkip True to only reserve the space of the 3D World block.
 */
void GameDataFile::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    GameDataFileBase::writeToStream(pStream, isSkip);
    pStream->writeS32(sizeof(GameDataFileSaveData));

    if (isSkip) {
        pStream->skip(sizeof(GameDataFileSaveData));
    } else {
        GameDataFileSaveData data = {};
        data.mPlayerLife = mPlayerLife;
        data.mLastPlayCourseId = mLastPlayCourseId;
        data.mRetryCount = mRetryCount;
        data.mUnknown10 = mUnknown98;
        data.mCasinoRoomCounter = mCasinoRoomCounter;
        data.mGoldenExpressCounter = mGoldenExpressCounter;
        data.mGameFlag = mGameFlag;
        data.mMiiverseSetting = mMiiverseSetting;
        data.mGhostSetting = mGhostSetting;
        data.mNetworkSetting = mNetworkSetting;
        data.mUnknown26 = mUnknownAC;
        data.mIsStartEnding = mIsStartEnding;
        data.mUnknown38 = mUnknown110;
        data.mMissCount = mMissCount;

        for (int i = 0; i < 4; i++) {
            data.mStockItems[i] = mpStockItems->getStockItem(i);
        }

        pStream->writeMemBlock(&data, sizeof(data));
    }

    mpCourseInfoHolder->writeToStream(pStream, isSkip);
    mpWorldGameDataHolder->writeToStream(pStream, isSkip);
}
