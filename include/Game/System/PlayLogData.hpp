#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
class PlayLogData {
  public:
    explicit PlayLogData(GameDataHolder* pHolder);
    void initializeData();
    u32* getCommonData();
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
