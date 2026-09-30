#pragma once
#include <nn/atk/atk_GroupFile.h>

namespace nn::atk::detail {
class GroupFileReader {
public:
    explicit GroupFileReader(const void* file);
    bool ReadGroupItemLocationInfo(GroupItemLocationInfo* info, u32 index) const;
    u32 GetGroupItemExCount() const;
    bool ReadGroupItemInfoEx(GroupFile::GroupItemInfoEx* info, u32 index) const;
private:
    const GroupFile::InfoBlockBody* mInfo;
    const void* mFileData;
    const GroupFile::InfoExBlockBody* mInfoEx;
};
static_assert(sizeof(GroupFileReader) == 0x18, "GroupFileReader size");
}
