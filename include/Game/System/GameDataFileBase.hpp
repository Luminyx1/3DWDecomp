#pragma once
#include <basis/seadTypes.h>
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
    virtual void initializeData() = 0;
    virtual bool isNewFile() const;
    virtual void onSave() = 0;
    virtual void startStage(int worldId, int stageId) = 0;
    virtual void onStageStart();
    virtual void onStageEnd();
    virtual void restartStage() = 0;
    virtual int calcClearStarLevel() = 0;
    virtual bool isTwinkleData() const = 0;
    virtual bool readFromStream(sead::ReadStream* pStream);
    virtual void writeToStream(sead::WriteStream* pStream, bool option) const;
    virtual sead::DateTime getLastPlayingTime() const;
    virtual bool entryPlayer(int userId, int characterType);
    virtual void startOpening();
    virtual void startSave();
    virtual void initPlayerLife(int life);
    virtual int getPlayerLife() const;
    virtual bool addPlayerLife(int life);
    const ControlUserData* getControlUserData(int userId) const;
    int getMainPlayerCharacterType() const;
    int getMainPlayerCharacterTypeSaved() const;
    bool addCoin(int count);

  protected:
    GameDataHolder* mpHolder;
    int mFileId;
    bool mNewFile;
    ControlUserDataHolder* mpUsers;
    bool mStageStarted;
    int mMainUserId;
    int mCoinCount;
    sead::DateTime mLastPlayingTime;
    sead::DateTime mCreationTime;
    u64 mUnknown40;
    u64 mUnknown48;
    u8 mUuid[16];
    char mUnknown60[16];
    sead::SafeString mName;
};
static_assert(sizeof(GameDataFileBase) == 0x80);
