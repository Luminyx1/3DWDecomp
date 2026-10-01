#include <nn/atk/atk_GroupFileReader.h>

namespace nn::atk::detail {
// file points to a complete FGRP resource; invalid headers or blocks leave the reader empty.
GroupFileReader::GroupFileReader(const void* file)
    : mInfo(nullptr), mFileData(nullptr), mInfoEx(nullptr) {
    auto* header = static_cast<const GroupFile::FileHeader*>(file);

    if (header->signature != 0x50524746 || header->byteOrder != 0xfeff || header->version != 0x10000) return;
    const auto* info = header->GetInfoBlock();
    const auto* data = header->GetFileBlock();
    const auto* extra = header->GetInfoExBlock();

    if (info == nullptr || data == nullptr || info->signature != 0x4f464e49 || data->signature != 0x454c4946) return;

    if (extra != nullptr) {
        if (extra->signature != 0x58464e49) return;
        mInfoEx = &extra->body;
    }

    mInfo = &info->body;
    mFileData = data->data;
}

// info receives the file identifier and embedded address; index selects a group item.
bool GroupFileReader::ReadGroupItemLocationInfo(GroupItemLocationInfo* info, u32 index) const {
    if (mInfo == nullptr) return false;

    if (mInfo->count <= index) return false;
    const auto* item = reinterpret_cast<const GroupFile::GroupItemInfo*>(reinterpret_cast<const u8*>(mInfo) + mInfo->items[index].offset);

    if (item == nullptr) return false;
    info->fileId = item->fileId;
    info->address = item->GetFileAddress(mFileData);
    return true;
}

u32 GroupFileReader::GetGroupItemExCount() const { return (mInfoEx != nullptr) ? mInfoEx->count : 0; }
// info receives the extended item record; index selects an entry in the optional INFX block.
bool GroupFileReader::ReadGroupItemInfoEx(GroupFile::GroupItemInfoEx* info, u32 index) const {
    if (mInfoEx == nullptr) return false;

    if (mInfoEx->count <= index) return false;
    const auto* item = reinterpret_cast<const GroupFile::GroupItemInfoEx*>(reinterpret_cast<const u8*>(mInfoEx) + mInfoEx->items[index].offset);

    if (item == nullptr) return false;
    *info = *item;
    return true;
}

const GroupFile::InfoBlock* GroupFile::FileHeader::GetInfoBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x7800) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const InfoBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}

const GroupFile::FileBlock* GroupFile::FileHeader::GetFileBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x7801) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const FileBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}

const GroupFile::InfoExBlock* GroupFile::FileHeader::GetInfoExBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x7802) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const InfoExBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}
}
