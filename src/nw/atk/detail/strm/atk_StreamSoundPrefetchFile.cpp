#include <nn/atk/atk_StreamSoundPrefetchFile.h>

namespace nn::atk::detail {
const StreamSoundPrefetchFile::InfoBlock* StreamSoundPrefetchFile::FileHeader::GetInfoBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4000) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const InfoBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }
    return nullptr;
}
const StreamSoundPrefetchFile::RegionBlock* StreamSoundPrefetchFile::FileHeader::GetRegionBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4003) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const RegionBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }
    return nullptr;
}
const StreamSoundPrefetchFile::PrefetchDataBlock* StreamSoundPrefetchFile::FileHeader::GetPrefetchDataBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4004) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const PrefetchDataBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }
    return nullptr;
}
u32 StreamSoundPrefetchFile::FileHeader::GetPrefetchDataBlockSize() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4004) return blocks[i].size;
    return 0;
}
bool StreamSoundPrefetchFile::FileHeader::HasRegionBlock() const { return GetRegionBlock() != nullptr; }
u32 StreamSoundPrefetchFile::FileHeader::GetRegionBlockSize() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4003) return blocks[i].size;
    return 0;
}
u32 StreamSoundPrefetchFile::FileHeader::GetRegionBlockOffset() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x4003) return blocks[i].offset;
    return 0;
}
const StreamSoundPrefetchFile::PrefetchSample* StreamSoundPrefetchFile::PrefetchData::GetPrefetchSample() const {
    return reinterpret_cast<const PrefetchSample*>(reinterpret_cast<const u8*>(this) + sample.offset);
}
const void* StreamSoundPrefetchFile::PrefetchSample::GetSampleAddress() const { return samples; }
}
