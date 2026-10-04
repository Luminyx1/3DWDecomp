#include "System/SaveDataAccessSequence.hpp"
#include "Layout/WindowProcessing.hpp"
#include "Layout/WindowSave.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/SaveData/SaveDataFunction.hpp"
#include "Project/Base/StringUtil.hpp"
#include "System/GameDataConst.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataHolder.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "Util/ControlUserUtil.hpp"

namespace {
NERVE_DECL(SaveDataAccessSequence, Idle)
NERVE_DECL(SaveDataAccessSequence, PreError)
NERVE_DECL(SaveDataAccessSequence, Init)
NERVE_DECL(SaveDataAccessSequence, ReadGame)
NERVE_DECL(SaveDataAccessSequence, WriteGame)

/**
 * @brief Write nerve that skips the currently playing file; shares exeWriteGame().
 */
class SaveDataAccessSequenceNrvWriteGameSkipPlayingFile : public al::Nerve {
  public:
    /**
     * @brief Run the write state on the owning sequence.
     * @param pKeeper Nerve keeper whose parent is the SaveDataAccessSequence.
     */
    void execute(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<SaveDataAccessSequence>()->exeWriteGame();
    }
};

NERVE_DECL(SaveDataAccessSequence, ProcessEnd)
NERVE_DECL(SaveDataAccessSequence, Flush)
NERVE_DECL(SaveDataAccessSequence, Error)
NERVE_DECL(SaveDataAccessSequence, Result)

// Non-const so that LLVM's global merging packs the nerves used together into one block, which
// the target relies on (startWriteNoWindow() selects between two adjacent nerves).
SaveDataAccessSequenceNrvIdle NrvSaveDataAccessSequenceIdle;
SaveDataAccessSequenceNrvPreError NrvSaveDataAccessSequencePreError;
SaveDataAccessSequenceNrvInit NrvSaveDataAccessSequenceInit;
SaveDataAccessSequenceNrvReadGame NrvSaveDataAccessSequenceReadGame;
SaveDataAccessSequenceNrvWriteGame NrvSaveDataAccessSequenceWriteGame;
SaveDataAccessSequenceNrvWriteGameSkipPlayingFile
    NrvSaveDataAccessSequenceWriteGameSkipPlayingFile;
SaveDataAccessSequenceNrvProcessEnd NrvSaveDataAccessSequenceProcessEnd;
SaveDataAccessSequenceNrvFlush NrvSaveDataAccessSequenceFlush;
SaveDataAccessSequenceNrvError NrvSaveDataAccessSequenceError;
SaveDataAccessSequenceNrvResult NrvSaveDataAccessSequenceResult;

const char cSaveFileName[] = "GameData.bin";
const u32 cSaveFileSize = 0x10000;
const u32 cSaveFileVersion = 0;
} // namespace

/**
 * @brief Construct the save sequence and its processing / result windows.
 * @param pHolder Game-data holder whose data is read from and written to the save file.
 * @param pErrorViewer Viewer used to report file-system errors.
 * @param pNetworkSystem Network system (unused by this build's sequence).
 * @param rInfo Layout init info used to create the windows.
 */
SaveDataAccessSequence::SaveDataAccessSequence(GameDataHolder* pHolder,
                                               al::ErrorViewer* pErrorViewer,
                                               al::NetworkSystem* pNetworkSystem,
                                               const al::LayoutInitInfo& rInfo)
    : al::NerveExecutor("セーブデータアクセスシーケンス"), mpHolder(pHolder), mPadPort(-1),
      mpErrorViewer(pErrorViewer), mpNetworkSystem(pNetworkSystem), mUnknown30(0),
      mGhostWorldIndex(0), mUnknown38(nullptr), mpWindowProcessing(nullptr),
      mpWindowSave(nullptr), mpGhostWorldList(nullptr), mUnknown58(0), mSaveDisabled(false),
      mShowMessage(false), mShowWindow(false), mDevelop(false) {
    mpGhostWorldList = new s32[GameDataFunction::getWorldNum(GameDataHolderAccessor(pHolder))];
    mpWindowProcessing = new WindowProcessing(rInfo, nullptr);
    mpWindowSave = new WindowSave(rInfo);
    initNerve(&NrvSaveDataAccessSequenceIdle, 0);
}

/**
 * @brief Step the sequence once.
 * @param isSuppressError True to hold back the error state while it is set.
 * @return True when the sequence is idle afterwards.
 */
bool SaveDataAccessSequence::update(bool isSuppressError) {
    mIsSuppressError = isSuppressError;
    updateNerve();
    return isDone();
}

/**
 * @brief Check whether no save operation is in progress.
 * @return True when the sequence is idle.
 */
bool SaveDataAccessSequence::isDone() const {
    return al::isNerve(this, &NrvSaveDataAccessSequenceIdle);
}

/**
 * @brief Check whether the sequence is waiting to show a save error.
 * @return True while in the pre-error state.
 */
bool SaveDataAccessSequence::isWaitShowError() const {
    return al::isNerve(this, &NrvSaveDataAccessSequencePreError);
}

/**
 * @brief Start initialising the save directory unless it already is.
 */
void SaveDataAccessSequence::startInit() {
    if (al::isInitializedSaveData()) {
        return;
    }

    al::setNerve(this, &NrvSaveDataAccessSequenceInit);
}

/**
 * @brief Initialise the save directory synchronously, disabling saves on failure.
 */
void SaveDataAccessSequence::startInitSync() {
    if (al::isInitializedSaveData()) {
        return;
    }

    al::initSaveDirSync(cSaveFileName, cSaveFileSize, cSaveFileVersion);
    if (!al::isSuccessSaveDataSequence()) {
        mSaveDisabled = true;
    }
}

/**
 * @brief Start reading the game save file.
 */
void SaveDataAccessSequence::startRead() {
    al::setNerve(this, &NrvSaveDataAccessSequenceReadGame);
}

/**
 * @brief Read the game save file synchronously while idle.
 */
void SaveDataAccessSequence::startReadSync() {
    if (!isDone()) {
        return;
    }

    al::readSaveDataSync(cSaveFileName, cSaveFileSize, cSaveFileVersion);
    if (al::isSuccessSaveDataSequence()) {
        mpHolder->readFromSaveDataBuffer();
    }
}

/**
 * @brief Start writing the game save file with the processing and result windows.
 * @param isSkipWaitWindowClose True to finish without waiting for the window to close.
 * @param padPort Pad port that owns the result window, or -1 to pick the first active user.
 */
void SaveDataAccessSequence::startWrite(bool isSkipWaitWindowClose, int padPort) {
    if (mSaveDisabled) {
        return;
    }

    mShowMessage = true;
    mShowWindow = true;
    mIsSkipWaitWindowClose = isSkipWaitWindowClose;
    mPadPort = padPort;
    al::setNerve(this, &NrvSaveDataAccessSequenceWriteGame);
}

/**
 * @brief Start writing the game save file with the processing window but no result window.
 */
void SaveDataAccessSequence::startWriteNoMessage() {
    if (mSaveDisabled) {
        return;
    }

    mShowMessage = false;
    mShowWindow = true;
    mIsSkipWaitWindowClose = false;
    al::setNerve(this, &NrvSaveDataAccessSequenceWriteGame);
}

/**
 * @brief Start writing the game save file without any window.
 * @param isSkipPlayingFile True to leave the currently playing file out of the write.
 */
void SaveDataAccessSequence::startWriteNoWindow(bool isSkipPlayingFile) {
    if (mSaveDisabled) {
        return;
    }

    mShowMessage = false;
    mShowWindow = false;
    mIsSkipWaitWindowClose = false;
    const al::Nerve* pNerve = &NrvSaveDataAccessSequenceWriteGame;
    if (isSkipPlayingFile) {
        pNerve = &NrvSaveDataAccessSequenceWriteGameSkipPlayingFile;
    }

    al::setNerve(this, pNerve);
}

/**
 * @brief Write the game save file synchronously while idle.
 */
void SaveDataAccessSequence::startWriteSync() {
    if (mSaveDisabled || !isDone()) {
        return;
    }

    mpHolder->writeToSaveDataBuffer(false);
    al::writeSaveDataSync(cSaveFileName, cSaveFileSize, cSaveFileVersion);
}

/**
 * @brief Idle state; nothing to do.
 */
void SaveDataAccessSequence::exeIdle() {}

/**
 * @brief Initialise the save directory, disabling saves and reporting an error on failure.
 */
void SaveDataAccessSequence::exeInit() {
    if (al::isFirstStep(this)) {
        al::requestInitSaveDir(cSaveFileName, cSaveFileSize, cSaveFileVersion);
    }

    if (!al::updateSaveDataSequence()) {
        return;
    }

    if (al::isSuccessSaveDataSequence()) {
        al::setNerve(this, &NrvSaveDataAccessSequenceIdle);
        return;
    }

    mSaveDisabled = true;
    al::setNerve(this, &NrvSaveDataAccessSequencePreError);
}

/**
 * @brief Read the game save file into the game-data holder.
 */
void SaveDataAccessSequence::exeReadGame() {
    if (al::isFirstStep(this)) {
        al::requestReadSaveData(cSaveFileName, cSaveFileSize, cSaveFileVersion);
    }

    if (!al::updateSaveDataSequence()) {
        return;
    }

    if (al::isSuccessSaveDataSequence()) {
        mpHolder->readFromSaveDataBuffer();
    } else if (al::getSaveDataSequenceResult() == 0x202) {
        if (mSaveDisabled) {
            mpHolder->writeToSaveDataBuffer(false);
            mpHolder->readFromSaveDataBuffer();
        }
    } else {
        al::setNerve(this, &NrvSaveDataAccessSequencePreError);
        return;
    }

    al::setNerve(this, &NrvSaveDataAccessSequenceIdle);
}

/**
 * @brief Read the ghost save file of the current ghost world (unused in this build).
 */
void SaveDataAccessSequence::exeReadGhost() {
    if (al::isFirstStep(this)) {
        al::StringTmp<32> fileName("GhostWorld%d", mGhostWorldIndex + 1);
        al::requestReadSaveData(fileName.cstr(), 0, GameDataConst::getSaveDataVersionGhost());
    }

    if (al::updateSaveDataSequence()) {
        al::isSuccessSaveDataSequence();
        al::setNerve(this, &NrvSaveDataAccessSequenceIdle);
    }
}

/**
 * @brief Report the ghost save-data size, which is unused in this build.
 * @param worldId World identifier; unused because ghost saves are disabled.
 * @return Zero bytes.
 */
int SaveDataAccessSequence::calcGhostSaveDataSize(int worldId) const {
    return 0;
}

/**
 * @brief Serialise the game data and write it to the save file.
 */
void SaveDataAccessSequence::exeWriteGame() {
    if (al::isFirstStep(this)) {
        if (mShowWindow) {
            mpWindowProcessing->appearWithSystemMessage("SaveSequence", "WindowProcessing_Save", 0,
                                                        true);
        }

        mpHolder->writeToSaveDataBuffer(
            al::isNerve(this, &NrvSaveDataAccessSequenceWriteGameSkipPlayingFile));
        if (mDevelop) {
            al::setNerve(this, &NrvSaveDataAccessSequenceProcessEnd);
            return;
        }

        al::requestWriteSaveData(cSaveFileName, cSaveFileSize, cSaveFileVersion, false);
    }

    if (!al::updateSaveDataSequence()) {
        return;
    }

    if (al::isSuccessSaveDataSequence()) {
        al::setNerve(this, &NrvSaveDataAccessSequenceFlush);
    } else {
        al::setNerve(this, &NrvSaveDataAccessSequencePreError);
    }
}

/**
 * @brief Ghost writes are disabled in this build; go straight to flushing.
 */
void SaveDataAccessSequence::exeWriteGhost() {
    al::setNerve(this, &NrvSaveDataAccessSequenceFlush);
}

/**
 * @brief Commit the written save data to storage.
 */
void SaveDataAccessSequence::exeFlush() {
    if (al::isFirstStep(this)) {
        al::requestFlushSaveData();
    }

    if (!al::updateSaveDataSequence()) {
        return;
    }

    if (al::isSuccessSaveDataSequence()) {
        al::setNerve(this, &NrvSaveDataAccessSequenceProcessEnd);
    } else {
        al::setNerve(this, &NrvSaveDataAccessSequencePreError);
    }
}

/**
 * @brief Close the processing window and wait until the error may be shown.
 */
void SaveDataAccessSequence::exePreError() {
    if (al::isFirstStep(this) && mpWindowProcessing->isAlive()) {
        mpWindowProcessing->requestClose();
    }

    if (mpWindowProcessing->isAlive() || mIsSuppressError) {
        return;
    }

    al::getLastSaveDataFSErrorCode();
    al::setNerve(this, &NrvSaveDataAccessSequenceError);
}

/**
 * @brief Finish the failed operation without showing any further window.
 */
void SaveDataAccessSequence::exeError() {
    mShowMessage = false;
    mShowWindow = false;
    al::setNerve(this, &NrvSaveDataAccessSequenceProcessEnd);
}

/**
 * @brief Close the processing window, then show the result window or go idle.
 */
void SaveDataAccessSequence::exeProcessEnd() {
    if (mShowWindow) {
        if (al::isFirstStep(this)) {
            mpWindowProcessing->requestClose();
        }

        if (!mIsSkipWaitWindowClose && mpWindowProcessing->isAlive()) {
            return;
        }
    } else if (mShowMessage) {
        al::setNerve(this, &NrvSaveDataAccessSequenceResult);
        return;
    }

    al::setNerve(this, &NrvSaveDataAccessSequenceIdle);
}

/**
 * @brief Show the save-result window and go idle once it closes.
 */
void SaveDataAccessSequence::exeResult() {
    if (al::isFirstStep(this)) {
        s32 padPort = mPadPort;
        if (padPort < 0) {
            padPort = rc::calcPadPortByFirstActiveUser(GameDataHolderAccessor(mpHolder));
        }

        mpWindowSave->appearWindow(padPort);
        mPadPort = -1;
    }

    if (mpWindowSave->isAlive()) {
        return;
    }

    al::setNerve(this, &NrvSaveDataAccessSequenceIdle);
}

/**
 * @brief Enable or disable future save requests.
 * @param enabled True to allow saving; false to reject future write requests.
 */
void SaveDataAccessSequence::enableSave(bool enabled) {
    mSaveDisabled = !enabled;
}

/**
 * @brief Check whether the HOME button menu may be opened.
 * @return True while idle or while showing the result window.
 */
bool SaveDataAccessSequence::isEnableHomeButtonMenu() const {
    return isDone() || al::isNerve(this, &NrvSaveDataAccessSequenceResult);
}

/**
 * @brief Check whether the processing window is still being shown.
 * @return True while a write with a processing window has not finished it.
 */
bool SaveDataAccessSequence::isWindowProcessingActive() const {
    mpWindowProcessing->isEnd();
    return mShowWindow && !mpWindowProcessing->isEnd();
}

/**
 * @brief Build the list of worlds that have ghost data (unused in this build).
 */
void SaveDataAccessSequence::makeSaveGhostWorldList() {}
