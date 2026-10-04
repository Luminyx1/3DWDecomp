#pragma once
#include <nn/atk/atk_GroupFile.h>

namespace nn::atk::detail {
class GroupFileReader {
  public:
    explicit GroupFileReader(const void* file);
    bool ReadGroupItemLocationInfo(GroupItemLocationInfo* info, u32 index) const;
    /**
     * @brief Reads the number of embedded files from an initialized group reader.
     * @return Number of entries in the valid INFO block.
     */
    u32 GetGroupItemCount() const { return mInfo->count; }
    u32 GetGroupItemExCount() const;
    bool ReadGroupItemInfoEx(GroupFile::GroupItemInfoEx* info, u32 index) const;

  private:
    const GroupFile::InfoBlockBody* mInfo;
    const void* mFileData;
    const GroupFile::InfoExBlockBody* mInfoEx;
};
static_assert(sizeof(GroupFileReader) == 0x18, "GroupFileReader size");
} // namespace nn::atk::detail
