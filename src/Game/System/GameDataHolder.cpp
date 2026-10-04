#include "System/GameDataHolder.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/SaveData/SaveDataFunction.hpp"
#include "System/ControlUserDataHolder.hpp"
#include "System/Data/SingleModeData.hpp"
#include "System/Data/StageListHolder.hpp"
#include "System/GameDataCommon.hpp"
#include "System/GameDataFile.hpp"
#include "System/GameDataFlagFunction.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataPlayReportCommon.hpp"
#include "System/PlayLogFunction.hpp"
#include "System/SaveDataAccessSequence.hpp"
#include "Util/ControlUserUtil.hpp"
#include "Util/PlayerUtil.hpp"
#include <erepo/Types.h>
#include <stream/seadRamStream.h>

namespace {

/**
 * @brief Header written in front of the save-data buffer.
 */
struct SaveDataHeader {
    u32 mVersion;
    u32 mSize;
    u32 mReserved[2];
};

constexpr u32 cSaveDataVersion = 0x16;
constexpr u32 cSaveDataBufferSize = 0x10000;
constexpr s32 cFileNum = 4;

} // namespace

/**
 * @brief Create every save file, the common data and the course database.
 * @param pNetworkSystem Network system used by the save sequence.
 */
GameDataHolder::GameDataHolder(al::NetworkSystem* pNetworkSystem)
    : mpCommon(nullptr), mpPlayReportCommon(nullptr), mppFiles(nullptr), mpPlayingFile(nullptr),
      mpStageList(nullptr), mpIslandDataList(nullptr), mIsSaveDataRead(false), mUnknown51(false),
      mIsSkipStartSave(false), mSingleMode(false), mUnknown61(false), mUnknown62(true),
      mIs2PAssistMode(false), mIsMapEnabled(true), mIsDemoWasCancelled(false),
      mIsSaveRequested(false), mIsPhase0(false), mIsSceneRestart(false), mpSaveAccess(nullptr),
      mpNetwork(pNetworkSystem), mpPlayerHolder(nullptr), mpSceneObjHolder(nullptr),
      mpPlayReportManager(nullptr) {
    mUnknown6A = false;
    mUnknown6B = false;
    mUnknown6C = false;
    mIsEnablePlayReport = false;
    mPlayStartTime.setNow();

    mpStageList = new StageListHolder();
    mpCommon = new GameDataCommon(this);
    mpPlayReportCommon = new GameDataPlayReportCommon(this);

    mppFiles = new GameDataFile*[cFileNum];
    for (s32 i = 0; i < cFileNum; i++) {
        mppFiles[i] = new GameDataFile(this, i);
    }

    mppSingleFiles = new SingleModeData*[cFileNum];
    for (s32 i = 0; i < cFileNum; i++) {
        mppSingleFiles[i] = new SingleModeData(this, i);
    }

    setPlayingFileId(getLastPlayingFileId());
    setSingleModePlayingFileID(getLastSingleModePlayingFileID(), false);
    initializeData();
}

/**
 * @brief Select the active 3D World save file.
 * @param fileId Save-file slot from 0 through 3.
 */
void GameDataHolder::setPlayingFileId(int fileId) {
    mpPlayingFile = mppFiles[fileId];
    mpCommon->mValues[1] = fileId;
    mpPlayingFile->setPlayingFile();
}

/**
 * @brief Select the active Bowser's Fury save file.
 * @param fileId Save-file slot from 0 through 3.
 * @param isInitPlayTime True to restart the play-time measurement while in single mode.
 */
void GameDataHolder::setSingleModePlayingFileID(int fileId, bool isInitPlayTime) {
    mpSingleFile = mppSingleFiles[fileId];
    if (isInitPlayTime && mSingleMode) {
        GameDataFunction::initTotalPlayTimeSM(GameDataHolderAccessor(this), fileId);
    }

    mpCommon->mValues[2] = fileId;
}

/**
 * @brief Reset every save file and the common data to their defaults.
 */
void GameDataHolder::initializeData() {
    al::initRandomSeedByTick();
    al::initRandomSeedByTickNonSync();
    mpCommon->initializeData();
    mpPlayReportCommon->initializeData();
    GameDataFile::initCameraSettings();
    SingleModeData::sOptions = {false, false, -1, 1};

    for (s32 i = 0; i < cFileNum; i++) {
        mppFiles[i]->initializeData();
        mppSingleFiles[i]->initializeData();
    }

    setPlayingFileId(0);
    setSingleModePlayingFileID(0, false);
    mIsSaveDataRead = false;
    mPlayStartTime.setNow();
    mIs2PAssistMode = false;
    mUnknown6A = false;
}

/**
 * @brief Store the current figure type of every user.
 * @param figureType Figure type given to every alive user, or a negative value to keep the
 * figure type each player actor currently has.
 */
void GameDataHolder::updatePlayerFigures(int figureType) {
    if (mpPlayerHolder == nullptr) {
        return;
    }

    const al::LiveActor* pPlayer = al::getPlayerActor(mpPlayerHolder, 0);
    for (s32 i = 0; i < rc::getControlUserNumMax(); i++) {
        const al::LiveActor* pActor = rc::tryFindAlivePlayerActorFirstByUserId(pPlayer, i);
        if (pActor != nullptr) {
            rc::setControlUserFigureType(GameDataHolderWriter(this), i,
                                         figureType < 0 ? rc::getPlayerFigureType(pActor) :
                                                          figureType);
        } else {
            rc::setControlUserFigureType(GameDataHolderWriter(this), i,
                                         rc::getPlayerFigureTypeDefault());
        }
    }
}

/**
 * @brief Reset every 3D World save file.
 */
void GameDataHolder::initialize3DWorldData() {
    for (s32 i = 0; i < cFileNum; i++) {
        mppFiles[i]->initializeData();
    }
}

/**
 * @brief Validate the loaded save data, resetting it when it is broken.
 * @return True when every part of the save data is valid.
 */
bool GameDataHolder::checkValid() {
    bool isValid = mpCommon->checkValid() & mpPlayReportCommon->checkValid();
    for (s32 i = 0; i < cFileNum; i++) {
        isValid &= mppFiles[i]->checkValid();
    }

    if (!isValid) {
        initializeData();
    }

    return isValid;
}

/**
 * @brief Bind the scene-object holder.
 * @param pHolder Scene-object holder used by subsequent game-data operations.
 */
void GameDataHolder::setSceneObjHolder(al::SceneObjHolder* pHolder) { mpSceneObjHolder = pHolder; }

/**
 * @brief Create the save-data access sequence.
 * @param pErrorViewer Error viewer used to report save errors.
 * @param pNetworkSystem Network system used by the sequence.
 * @param rInfo Layout initialization info for the save windows.
 */
void GameDataHolder::createSaveDataAccessSequence(al::ErrorViewer* pErrorViewer,
                                                  al::NetworkSystem* pNetworkSystem,
                                                  const al::LayoutInitInfo& rInfo) {
    mpSaveAccess = new SaveDataAccessSequence(this, pErrorViewer, pNetworkSystem, rInfo);
}

/**
 * @brief Create the development variant of the save-data access sequence.
 * @param pErrorViewer Error viewer used to report save errors.
 * @param pNetworkSystem Network system used by the sequence.
 * @param rInfo Layout initialization info for the save windows.
 */
void GameDataHolder::createSaveDataAccessSequenceDevelop(al::ErrorViewer* pErrorViewer,
                                                         al::NetworkSystem* pNetworkSystem,
                                                         const al::LayoutInitInfo& rInfo) {
    mpSaveAccess = new SaveDataAccessSequence(this, pErrorViewer, pNetworkSystem, rInfo);
    mpSaveAccess->setDevelop();
}

/**
 * @brief Decide whether the Bowser's Fury prologue phase is active.
 */
void GameDataHolder::initIsPhase0() {
    const SingleModeData* pFile = mpSingleFile;
    mIsPhase0 = false;
    if (pFile != nullptr && pFile->getUnlockedPhase() == 0) {
        mIsPhase0 = true;
    }
}

/**
 * @brief Remember the last game mode.
 * @param mode Game mode to persist in common save data.
 */
void GameDataHolder::setLastPlayedMode(GameMode mode) {
    mpCommon->mValues[0] = static_cast<u32>(mode);
}

/**
 * @brief Read the last game mode.
 * @return The saved game-mode identifier.
 */
GameMode GameDataHolder::getLastPlayedMode() const {
    return static_cast<GameMode>(mpCommon->mValues[0]);
}

/**
 * @brief Initialize the play-report manager.
 * @param pName Unused report name.
 * @return True when a play-report manager exists.
 */
bool GameDataHolder::initializePlayReport(const char* pName) {
    if (mpPlayReportManager == nullptr) {
        return false;
    }

    mpPlayReportManager->Initialize(this, 100);
    return true;
}

/**
 * @brief Update the play-report manager once save data has been read.
 */
void GameDataHolder::updatePlayReport() {
    if (mIsEnablePlayReport && mpPlayReportManager != nullptr) {
        mpPlayReportManager->Update();
    }
}

/**
 * @brief Report the controller style of the first user.
 */
void GameDataHolder::updatePlayStyle() {
    GameDataFileBase* pFile;
    if (mSingleMode) {
        pFile = mpSingleFile;
    } else {
        pFile = getGameDataFile(getLastPlayingFileId());
    }

    const ControlUserData* pUser = pFile->getControlUserDataHolder()->getControlUserData(0);
    erepo::EControllerStyle style;
    if (al::isPadTypeFullKey(pUser->mPadPort)) {
        style = erepo::EControllerStyle::FullKey;
    } else if (al::isPadTypeHandheld(pUser->mPadPort)) {
        style = erepo::EControllerStyle::Handheld;
    } else if (al::isPadTypeJoyDual(pUser->mPadPort)) {
        style = erepo::EControllerStyle::Dual;
    } else if (al::isPadTypeJoySingle(pUser->mPadPort)) {
        if (al::isPadTypeJoyLeft(pUser->mPadPort) || al::isPadTypeJoyRight(pUser->mPadPort)) {
            style = erepo::EControllerStyle::Dual;
        } else {
            style = erepo::EControllerStyle::Unknown;
        }
    } else {
        style = erepo::EControllerStyle::Unknown;
    }

    if (mpPlayReportManager != nullptr) {
        mpPlayReportManager->UpdateStyle(style);
    }
}

/**
 * @brief Access a save-file slot.
 * @param fileId Save-file slot from 0 through 3.
 * @return The save data belonging to the requested slot.
 */
GameDataFile* GameDataHolder::getGameDataFile(int fileId) const { return mppFiles[fileId]; }

/**
 * @brief Begin a play-report event.
 * @param type Kind of the event.
 * @param eventId Event identifier.
 * @param option Event-specific option.
 * @return True when the event was started or no play-report manager exists.
 */
bool GameDataHolder::beginPlayReport(preport::KeyEventType type, int eventId, int option) {
    if (mpPlayReportManager == nullptr) {
        return true;
    }

    return mpPlayReportManager->EventBegin(type, eventId, option, true);
}

/**
 * @brief End the current play-report event.
 */
void GameDataHolder::endPlayReport() { mpPlayReportManager->EventEnd(); }

/**
 * @brief Record an integer value in the current play-report event.
 * @param key Key of the value.
 * @param value Value to record.
 */
void GameDataHolder::setPlayReportData(preport::Key key, int value) {
    mpPlayReportManager->KeySetValue(key, value);
}

/**
 * @brief Record a float value in the current play-report event.
 * @param key Key of the value.
 * @param value Value to record.
 */
void GameDataHolder::setPlayReportData(preport::Key key, float value) {
    mpPlayReportManager->KeySetValue(key, value);
}

/**
 * @brief Record a 64-bit value in the current play-report event.
 * @param key Key of the value.
 * @param value Value to record.
 */
void GameDataHolder::setPlayReportData(preport::Key key, s64 value) {
    mpPlayReportManager->KeySetValue(key, value);
}

/**
 * @brief Record a string in the current play-report event.
 * @param key Key of the value.
 * @param rValue String to record.
 */
void GameDataHolder::setPlayReportData(preport::Key key, sead::SafeString& rValue) {
    mpPlayReportManager->KeySetValue(key, rValue);
}

/**
 * @brief Record an integer array in the current play-report event.
 * @param key Key of the values.
 * @param pValues Values to record.
 * @param num Number of values.
 */
void GameDataHolder::setPlayReportData(preport::Key key, int* pValues, int num) {
    mpPlayReportManager->KeySetValue(key, pValues, num);
}

/**
 * @brief Record a float array in the current play-report event.
 * @param key Key of the values.
 * @param pValues Values to record.
 * @param num Number of values.
 */
void GameDataHolder::setPlayReportData(preport::Key key, float* pValues, int num) {
    mpPlayReportManager->KeySetValue(key, pValues, num);
}

/**
 * @brief Send the network status play report.
 */
void GameDataHolder::sendNetworkStatus() { mpPlayReportManager->sendNetworkStatus(); }

/**
 * @brief Start a new play-report session.
 */
void GameDataHolder::addSessionId() { mpPlayReportManager->addSessionId(); }

/**
 * @brief Read the last selected save-file slot.
 * @return The saved slot index.
 */
int GameDataHolder::getLastPlayingFileId() const { return mpCommon->mValues[1]; }

/**
 * @brief Check whether a save-file slot has never been played.
 * @param fileId Save-file slot from 0 through 3.
 * @return True for a fresh file of the current mode.
 */
bool GameDataHolder::isNewFile(int fileId) const {
    if (mSingleMode) {
        return mppSingleFiles[fileId]->isNewFile();
    }

    return mppFiles[fileId]->isNewFile();
}

/**
 * @brief Access a save-file slot.
 * @param fileId Save-file slot from 0 through 3.
 * @return The save data belonging to the requested slot.
 */
SingleModeData* GameDataHolder::getSingleModeDataFile(int fileId) const {
    return mppSingleFiles[fileId];
}

/**
 * @brief Copy a 3D World save file into another slot.
 * @param srcFileId Slot to copy from.
 * @param dstFileId Slot to copy to.
 */
void GameDataHolder::copySaveFile(int srcFileId, int dstFileId) {
    mppFiles[dstFileId]->copySaveFile(*mppFiles[srcFileId]);
}

/**
 * @brief Reset a save file of the current mode.
 * @param fileId Save-file slot from 0 through 3.
 */
void GameDataHolder::deleteSaveFile(int fileId) {
    if (mSingleMode) {
        mppSingleFiles[fileId]->initializeData();
        return;
    }

    mppFiles[fileId]->initializeData();
}

/**
 * @brief Check whether Luigi Bros. has been unlocked.
 * @return True when the common unlock flag is set.
 */
bool GameDataHolder::isUnlockLuigiBros() const { return mpCommon->mState; }

/**
 * @brief Unlock Luigi Bros. in common save data.
 */
void GameDataHolder::unlockLuigiBros() { mpCommon->mState = true; }

/**
 * @brief Check whether the active Bowser's Fury file has never been played.
 * @return True for a fresh single-mode file.
 */
bool GameDataHolder::isNewFile() const { return mpSingleFile->isNewFile(); }

/**
 * @brief Read the last selected save-file slot.
 * @return The saved slot index.
 */
int GameDataHolder::getLastSingleModePlayingFileID() const { return mpCommon->mValues[2]; }

/**
 * @brief Copy a Bowser's Fury save file into another slot.
 * @param srcFileId Slot to copy from.
 * @param dstFileId Slot to copy to.
 */
void GameDataHolder::copySingleModeFile(int srcFileId, int dstFileId) {
    mppSingleFiles[dstFileId]->copySingleModeFile(*mppSingleFiles[srcFileId]);
}

/**
 * @brief Access the stage data of the active 3D World file.
 * @return The stage data holder.
 */
const StageDataHolder* GameDataHolder::getStageDataHolder() const {
    return mpPlayingFile->getStageDataHolder();
}

/**
 * @brief Access the stage data of the active 3D World file.
 * @return The mutable stage data holder.
 */
StageDataHolder* GameDataHolder::getStageDataHolderPtr() {
    return mpPlayingFile->getStageDataHolderPtr();
}

/**
 * @brief Read the user with the best score in the last played stage.
 * @return The user identifier.
 */
int GameDataHolder::tryGetLastStageBestScoreUserID() const {
    return mpPlayingFile->tryGetLastStageBestScoreUserID();
}

/**
 * @brief Start the opening of the active file of the current mode.
 */
void GameDataHolder::startOpening() {
    if (mSingleMode) {
        mpSingleFile->startOpening();
        return;
    }

    mpPlayingFile->startOpening();
}

/**
 * @brief Start the ending of the active 3D World file.
 */
void GameDataHolder::startEnding() { mpPlayingFile->startEnding(); }

/**
 * @brief Check whether the active 3D World file is played for the first time.
 * @return True for a fresh file.
 */
bool GameDataHolder::isFirstPlay() const { return mpPlayingFile->isNewFile(); }

/**
 * @brief Access the course info of the active 3D World file.
 * @param courseId Course identifier.
 * @return The course info.
 */
const CourseInfo* GameDataHolder::getCourseInfo(int courseId) const {
    return mpPlayingFile->getCourseInfo(courseId);
}

/**
 * @brief Access the course info of the active 3D World file.
 * @param courseId Course identifier.
 * @return The mutable course info.
 */
CourseInfo* GameDataHolder::getCourseInfoPtr(int courseId) {
    return mpPlayingFile->getCourseInfoPtr(courseId);
}

/**
 * @brief Access the control-user data of the active file of the current mode.
 * @return The control-user data holder.
 */
ControlUserDataHolder* GameDataHolder::getControlUserDataHolder() const {
    return mSingleMode ? mpSingleFile->getControlUserDataHolder() :
                         mpPlayingFile->getControlUserDataHolder();
}

/**
 * @brief Count the selectable character types.
 * @return 5 once Rosalina is playable, otherwise 4.
 */
int GameDataHolder::calcCharacterTypeNumMax() const {
    return GameDataFlagFunction::isAlreadyOpenRosetta(
               GameDataHolderAccessor(const_cast<GameDataHolder*>(this))) ?
               5 :
               4;
}

/**
 * @brief Load every save file from the save-data work buffer.
 */
void GameDataHolder::readFromSaveDataBuffer() {
    mIsEnablePlayReport = true;
    sead::RamReadStream stream(al::getSaveDataWorkBuffer(), cSaveDataBufferSize,
                               sead::Stream::Modes::Binary);
    mPlayStartTime.setNow();
    initializeData();
    setPlayingFileId(0);
    setSingleModePlayingFileID(0, false);
    mIsSaveDataRead = true;

    SaveDataHeader header = {};
    stream.readMemBlock(&header, sizeof(header));
    if (header.mVersion != cSaveDataVersion) {
        initializeData();
        mIsSaveDataRead = true;
        return;
    }

    if (!mpCommon->readFromStream(&stream)) {
        initializeData();
        mIsSaveDataRead = true;
        return;
    }

    u8 cameraSettings;
    stream.readMemBlock(&cameraSettings, sizeof(cameraSettings));
    GameDataFile::sCameraSettings = cameraSettings;
    for (s32 i = 0; i < cFileNum; i++) {
        if (!mppFiles[i]->readFromStream(&stream)) {
            initializeData();
            mIsSaveDataRead = true;
            return;
        }
    }

    SingleModeData::Options options = {false, false, -1, 1};
    stream.readMemBlock(&options, sizeof(options));
    SingleModeData::sOptions = options;
    for (s32 i = 0; i < cFileNum; i++) {
        if (!mppSingleFiles[i]->readFromStream(&stream)) {
            initializeData();
            mIsSaveDataRead = true;
            return;
        }
    }

    if (mpCommon->mPlayReportVersion == 1) {
        mpPlayReportCommon->readFromStream(&stream);
    } else {
        mpPlayReportCommon->initializeData();
        mpCommon->updatePlayReportCommonVersion();
    }

    if (header.mSize != stream.getSrc().getCurrentPos()) {
        initializeData();
        mIsSaveDataRead = true;
        return;
    }

    checkValid();
    setPlayingFileId(getLastPlayingFileId());
    setSingleModePlayingFileID(getLastSingleModePlayingFileID(), false);
}

/**
 * @brief Store every save file into the save-data work buffer.
 * @param isSkipPlayingFile True to leave the active file out of the save.
 */
void GameDataHolder::writeToSaveDataBuffer(bool isSkipPlayingFile) {
    sead::DateTime prevTime = mPlayStartTime;
    mPlayStartTime.setNow();
    PlayLogFunction::setPlayTime(GameDataHolderWriter(this),
                                 mPlayStartTime.diff(prevTime).getSpan());
    if (!isSkipPlayingFile) {
        if (mSingleMode && mpSingleFile != nullptr) {
            mpSingleFile->startSave();
        } else if (!mIsSkipStartSave && mpPlayingFile != nullptr) {
            mpPlayingFile->startSave();
        }
    }

    u8* pBuffer = al::getSaveDataWorkBuffer();
    sead::RamWriteStream stream(pBuffer, cSaveDataBufferSize, sead::Stream::Modes::Binary);
    SaveDataHeader header = {};
    header.mVersion = cSaveDataVersion;

    stream.writeMemBlock(&header, sizeof(header));
    mpCommon->updatePlayReportCommonVersion();
    mpCommon->writeToStream(&stream);

    u8 cameraSettings = GameDataFile::sCameraSettings;
    stream.writeMemBlock(&cameraSettings, sizeof(cameraSettings));
    for (s32 i = 0; i < cFileNum; i++) {
        bool isSkip = isSkipPlayingFile && !mSingleMode && mpPlayingFile->getFileId() == i;
        mppFiles[i]->writeToStream(&stream, isSkip);
        if (!isSkip) {
            mppFiles[i]->onSave();
        }
    }

    SingleModeData::Options options = SingleModeData::sOptions;
    stream.writeMemBlock(&options, sizeof(options));
    for (s32 i = 0; i < cFileNum; i++) {
        bool isSkip = isSkipPlayingFile && mSingleMode && mpSingleFile->getFileId() == i;
        mppSingleFiles[i]->writeToStream(&stream, isSkip);
        if (!isSkip) {
            mppSingleFiles[i]->onSave();
        }
    }

    mpPlayReportCommon->writeToStream(&stream);
    reinterpret_cast<SaveDataHeader*>(pBuffer)->mSize = stream.getSrc().getCurrentPos();
}

/**
 * @brief Select the game data used by the title demonstration.
 * @param pFile Game-data file used while the title demonstration runs.
 */
void GameDataHolder::setGameFileForTitleDemo(GameDataFile* pFile) { mpPlayingFile = pFile; }

/**
 * @brief Return to the last selected file after the title demonstration.
 */
void GameDataHolder::resetGameFileForTitleDemo() { setPlayingFileId(getLastPlayingFileId()); }

/**
 * @brief Identify the game-data scene object.
 * @return The scene-object name.
 */
const char* GameDataHolder::getSceneObjName() const {
    return "ゲームデータ保持";
}
