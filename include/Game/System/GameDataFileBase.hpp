#pragma once
#include <basis/seadTypes.h>
#include <nn/time.h>
#include <nn/util/util_Uuid.h>
#include <prim/seadSafeString.h>
#include <time/seadDateTime.h>
class GameDataHolder;
class ControlUserDataHolder;
class ControlUserData;
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
class GameDataFileBase {
  public:
    GameDataFileBase(GameDataHolder* pHolder, int fileId, bool isSingleMode);
    virtual void initializeData() = 0;

    /**
     * @brief Check whether the file has never been played.
     * @return True for a fresh file.
     */
    virtual bool isNewFile() const { return mNewFile; }

    virtual void onSave() = 0;
    virtual void startStage(int worldId, int stageId) = 0;
    virtual void onStageStart();
    virtual void onStageEnd();
    virtual void restartStage() = 0;
    virtual int calcClearStarLevel() = 0;
    virtual bool isTwinkleData() const = 0;
    virtual bool readFromStream(sead::ReadStream* pStream);
    virtual void writeToStream(sead::WriteStream* pStream, bool option) const;

    /**
     * @brief Read when the file was last played.
     * @return The last playing time.
     */
    virtual sead::DateTime getLastPlayingTime() const { return mLastPlayingTime; }

    virtual bool entryPlayer(int userId, int characterType);
    virtual void startOpening();
    virtual void startSave();
    virtual void initPlayerLife(int life);
    virtual int getPlayerLife() const;
    virtual int addPlayerLife(int life);
    void initializeData(bool isSingleMode);
    void copyFileBase(const GameDataFileBase& rOther);
    const ControlUserData* getControlUserData(int userId) const;
    int getMainPlayerCharacterType() const;
    int getMainPlayerCharacterTypeSaved() const;
    bool addCoin(int count);
    void updateTotalPlayTimePR();
    void initTotalPlayTimePR();
    s64 getTotalPlayTimePR(bool isIncludeCurrentPlay);

    /**
     * @brief Access the save-file name.
     * @return The save-file name.
     */
    sead::SafeString& getName() { return mName; }

    /**
     * @brief Read the save-file slot of this file.
     * @return The save-file slot index.
     */
    int getFileId() const { return mFileId; }

    /**
     * @brief Read the number of collected coins.
     * @return The coin count.
     */
    int getCoinNum() const { return mCoinCount; }

    /**
     * @brief Access the per-user control data of this file.
     * @return The control-user data holder.
     */
    ControlUserDataHolder* getControlUserDataHolder() const { return mpUsers; }

  protected:
    GameDataHolder* mpHolder;
    int mFileId;
    bool mNewFile;
    ControlUserDataHolder* mpUsers;
    bool mStageStarted;
    int mMainUserId;
    int mCoinCount;
    sead::DateTime mLastPlayingTime;
    sead::DateTime mPlayStartTime;
    nn::TimeSpan mPlayStartActiveTime;
    s64 mTotalPlayTime;
    nn::util::Uuid mUuid;
    char mUuidString[16];
    sead::SafeString mName;
};
static_assert(sizeof(GameDataFileBase) == 0x80);
