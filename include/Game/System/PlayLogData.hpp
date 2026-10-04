#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
/**
 * @brief Layout of the common (non-course) play-log counters.
 */
struct PlayLogCommonData {
    u32 mPlayCount;               // 0x00
    u32 mPlayTime;                // 0x04
    u32 mMarioCount;              // 0x08
    u32 mLuigiCount;              // 0x0C
    u32 mPeachCount;              // 0x10
    u32 mKinopioCount;            // 0x14
    u32 mRosettaCount;            // 0x18
    u32 mUserNum1Count;           // 0x1C
    u32 mUserNum2Count;           // 0x20
    u32 mUserNum3Count;           // 0x24
    u32 mUserNum4Count;           // 0x28
    u32 mUseAssistPlayerCount;    // 0x2C
    u32 mUnknown30[5];            // 0x30
    u32 mUseCrossKeyCount;        // 0x44
    u32 mUseTouchPanelCount;      // 0x48
    u32 mUseCameraRotateCount;    // 0x4C
    u32 mMiiverseFlag;            // 0x50
    u32 mPostMiiverseCount;       // 0x54
    u32 mPlayerEntryCount;        // 0x58
};

class PlayLogData {
  public:
    explicit PlayLogData(GameDataHolder* pHolder);
    void initializeData();
    PlayLogCommonData* getCommonData();
    u32* tryGetCourseData(int courseId);
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream) const;
    /**
     * @brief Read the number of counters used by the loaded course table.
     * @return The common and per-course counter count.
     */
    int getValueCount() const { return mUsedValueCount; }

    /**
     * @brief Access the raw play-log counters.
     * @return The allocated counter buffer.
     */
    u32* getValues() const { return mpValues; }

  private:
    GameDataHolder* mpHolder;
    u32* mpValues;
    s32 mUsedValueCount;
    s32* mpCourseOffsets;
    s32 mCourseCount;
};
static_assert(sizeof(PlayLogData) == 0x28);
