#include "System/GameDataFileBase.hpp"
#include "System/ControlUserDataHolder.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "Util/ControlUserUtil.hpp"
#include <cstring>
#include <nn/oe.h>
#include <nn/util/util_FormatString.h>
#include <stream/seadStream.h>

namespace {

/**
 * @brief Serialized layout of the common save-file block.
 */
struct FileBaseSaveData {
    bool mIsNewFile = true;
    s32 mMainUserId;
    s32 mCoinCount;
    sead::DateTime mLastPlayingTime;
    sead::DateTime mPlayStartTime;
    s64 mTotalPlayTime;
    nn::util::Uuid mUuid;
    u64 mReserved;
};
static_assert(sizeof(FileBaseSaveData) == 0x40);

/**
 * @brief Format a UUID as text into a fixed buffer.
 * @param pBuffer Destination buffer of 16 characters; the text is truncated to fit.
 * @param rUuid UUID to format.
 */
inline void formatUuid(char* pBuffer, const nn::util::Uuid& rUuid) {
    nn::util::TSNPrintf(pBuffer, 16,
                        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        rUuid.data[0], rUuid.data[1], rUuid.data[2], rUuid.data[3], rUuid.data[4],
                        rUuid.data[5], rUuid.data[6], rUuid.data[7], rUuid.data[8], rUuid.data[9],
                        rUuid.data[10], rUuid.data[11], rUuid.data[12], rUuid.data[13],
                        rUuid.data[14], rUuid.data[15]);
}

} // namespace

/**
 * @brief Construct a fresh save file and allocate its control-user records.
 * @param pHolder Game-data holder owning the file.
 * @param fileId Index of the save file.
 * @param isSingleMode True when the file belongs to the single-player mode.
 */
GameDataFileBase::GameDataFileBase(GameDataHolder* pHolder, int fileId, bool isSingleMode)
    : mpHolder(pHolder), mFileId(fileId), mNewFile(true), mpUsers(nullptr), mStageStarted(false),
      mMainUserId(0), mCoinCount(0), mLastPlayingTime(0), mPlayStartTime(0),
      mPlayStartActiveTime(), mTotalPlayTime(0) {
    mpUsers = new ControlUserDataHolder(isSingleMode);
}

/**
 * @brief Reset the common file data and assign a new identifier.
 * @param isSingleMode True when the file belongs to the single-player mode.
 */
void GameDataFileBase::initializeData(bool isSingleMode) {
    mNewFile = true;
    mLastPlayingTime = sead::DateTime(0);
    mPlayStartTime.setNow();
    mTotalPlayTime = 0;
    mpUsers->initialize(isSingleMode);
    mStageStarted = false;
    mMainUserId = 0;
    mCoinCount = 0;
    mUuid = nn::util::GenerateUuid();
    formatUuid(mUuidString, mUuid);
    mName = sead::SafeString(mUuidString);
}

/**
 * @brief Update whether this file is playing a stage.
 */
void GameDataFileBase::onStageStart() { mStageStarted = true; }

/**
 * @brief Update whether this file is playing a stage.
 */
void GameDataFileBase::onStageEnd() { mStageStarted = false; }

/**
 * @brief Load the common file data from a save stream.
 * @param pStream Stream to read from.
 * @return True when the control-user records were read successfully.
 */
bool GameDataFileBase::readFromStream(sead::ReadStream* pStream) {
    s32 size;
    pStream->readS32(size);
    FileBaseSaveData save;
    std::memset(&save, 0, sizeof(save));
    pStream->readMemBlock(&save, sizeof(save));
    mNewFile = save.mIsNewFile;
    mMainUserId = save.mMainUserId;
    mCoinCount = save.mCoinCount;
    mLastPlayingTime = save.mLastPlayingTime;
    mPlayStartTime = save.mPlayStartTime;
    mTotalPlayTime = save.mTotalPlayTime;
    mUuid = save.mUuid;
    formatUuid(mUuidString, mUuid);
    mName = sead::SafeString(mUuidString);

    if (!mpUsers->readFromStream(pStream)) {
        return false;
    }

    initTotalPlayTimePR();
    return true;
}

/**
 * @brief Write the common file data to a save stream.
 * @param pStream Stream to write to.
 * @param isSkip True to only advance the stream past the data.
 */
void GameDataFileBase::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    pStream->writeS32(sizeof(FileBaseSaveData));

    if (isSkip) {
        pStream->skip(sizeof(FileBaseSaveData));
    } else {
        FileBaseSaveData save;
        std::memset(&save, 0, sizeof(save));
        save.mIsNewFile = mNewFile;
        save.mMainUserId = mMainUserId;
        save.mCoinCount = mCoinCount;
        save.mLastPlayingTime = mLastPlayingTime;
        save.mPlayStartTime = mPlayStartTime;
        save.mTotalPlayTime = mTotalPlayTime;
        save.mUuid = mUuid;
        pStream->writeMemBlock(&save, sizeof(save));
    }

    mpUsers->writeToStream(pStream, isSkip);
}

/**
 * @brief Access a player's persistent control-user record.
 * @param userId Control-user index from 0 through 3.
 * @return The selected control-user record.
 */
const ControlUserData* GameDataFileBase::getControlUserData(int userId) const {
    return mpUsers->getControlUserData(userId);
}

/**
 * @brief Enter a player with the requested character.
 * @param userId Control-user index from 0 through 3.
 * @param characterType Requested character; -1 preserves the current selection.
 * @return True when the character can be assigned.
 */
bool GameDataFileBase::entryPlayer(int userId, int characterType) {
    return mpUsers->entryPlayer(userId, characterType);
}

/**
 * @brief Read the main player's character selection.
 * @return The main player's character identifier.
 */
int GameDataFileBase::getMainPlayerCharacterType() const {
    return getControlUserData(mMainUserId)->mCharacterType;
}

/**
 * @brief Read the main player's character selection.
 * @return The main player's character identifier.
 */
int GameDataFileBase::getMainPlayerCharacterTypeSaved() const {
    return getControlUserData(mMainUserId)->mSavedCharacterType;
}

/**
 * @brief Mark the file as used and select the first active player as the main player.
 */
void GameDataFileBase::startOpening() {
    mNewFile = false;
    mLastPlayingTime.setNow();
    mMainUserId = rc::getActiveControlUserFirst(GameDataHolderAccessor(mpHolder));
}

/**
 * @brief Record the main player, save time, and accumulated play time before saving.
 */
void GameDataFileBase::startSave() {
    mMainUserId = rc::getActiveControlUserFirst(GameDataHolderAccessor(mpHolder));
    mLastPlayingTime.setNow();
    updateTotalPlayTimePR();
}

/**
 * @brief Add the time played since the last measurement to the total and restart measuring.
 */
void GameDataFileBase::updateTotalPlayTimePR() {
    nn::TimeSpan now = nn::oe::GetProgramTotalActiveTime();
    nn::TimeSpan elapsed =
        nn::TimeSpan::FromNanoSeconds(now.nanoseconds - mPlayStartActiveTime.nanoseconds);
    mTotalPlayTime += elapsed.GetSeconds();

    initTotalPlayTimePR();
}

/**
 * @brief Restart measuring play time from now.
 */
void GameDataFileBase::initTotalPlayTimePR() {
    mPlayStartTime.setNow();
    mPlayStartActiveTime = nn::oe::GetProgramTotalActiveTime();
}

/**
 * @brief Read the total play time in seconds.
 * @param isIncludeCurrentPlay True to first add the time played since the last measurement.
 * @return The total play time in seconds.
 */
s64 GameDataFileBase::getTotalPlayTimePR(bool isIncludeCurrentPlay) {
    if (isIncludeCurrentPlay) {
        updateTotalPlayTimePR();
    }

    return mTotalPlayTime;
}

/**
 * @brief Add coins and consume one hundred when the total crosses the threshold.
 * @param count Signed coins to add; this operation awards at most one life.
 * @return True when one hundred coins were consumed.
 */
bool GameDataFileBase::addCoin(int count) {
    mCoinCount += count;

    if (mCoinCount >= 100) {
        mCoinCount -= 100;
        return true;
    }
    return false;
}

/**
 * @brief Copy the common file data from another file.
 * @param rOther File to copy from.
 */
void GameDataFileBase::copyFileBase(const GameDataFileBase& rOther) {
    mNewFile = rOther.mNewFile;
    mpUsers->copy(rOther.mpUsers);
    mStageStarted = rOther.mStageStarted;
    mMainUserId = rOther.mMainUserId;
    mCoinCount = rOther.mCoinCount;
    mLastPlayingTime = rOther.mLastPlayingTime;
    mPlayStartTime = rOther.mPlayStartTime;
    mTotalPlayTime = rOther.mTotalPlayTime;
}

/**
 * @brief Handle life initialization for a mode without a life counter.
 * @param life Requested life count; unused in the base implementation.
 */
void GameDataFileBase::initPlayerLife(int life) {}

/**
 * @brief Read the base mode's unused life counter.
 * @return Zero; derived game modes may override this.
 */
int GameDataFileBase::getPlayerLife() const { return 0; }

/**
 * @brief Handle a life award for a mode without a life counter.
 * @param life Requested life increment; unused in the base implementation.
 * @return False because the base mode has no lives to update.
 */
bool GameDataFileBase::addPlayerLife(int life) { return false; }
