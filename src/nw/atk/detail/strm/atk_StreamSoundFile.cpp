#include <nn/atk/atk_StreamSoundFile.h>

namespace nn::atk::detail {
// type is the block tag to find; a missing reference returns null.
const ReferenceWithSize* StreamSoundFile::FileHeader::GetReferenceBy(u16 type) const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == type) return &blocks[i];
    return nullptr;
}

bool StreamSoundFile::FileHeader::HasSeekBlock() const { return GetReferenceBy(0x4001) != nullptr; }
bool StreamSoundFile::FileHeader::HasRegionBlock() const { return GetReferenceBy(0x4003) != nullptr; }
bool StreamSoundFile::FileHeader::HasMarkerBlock() const { return GetReferenceBy(0x4005) != nullptr; }

// Size and offset accessors require the requested block to exist in the file.
u32 StreamSoundFile::FileHeader::GetInfoBlockSize() const { return GetReferenceBy(0x4000)->size; }
u32 StreamSoundFile::FileHeader::GetSeekBlockSize() const { return GetReferenceBy(0x4001)->size; }
u32 StreamSoundFile::FileHeader::GetDataBlockSize() const { return GetReferenceBy(0x4002)->size; }
u32 StreamSoundFile::FileHeader::GetRegionBlockSize() const { return GetReferenceBy(0x4003)->size; }
u32 StreamSoundFile::FileHeader::GetMarkerBlockSize() const { return GetReferenceBy(0x4005)->size; }
u32 StreamSoundFile::FileHeader::GetInfoBlockOffset() const { return GetReferenceBy(0x4000)->offset; }
u32 StreamSoundFile::FileHeader::GetSeekBlockOffset() const { return GetReferenceBy(0x4001)->offset; }
u32 StreamSoundFile::FileHeader::GetDataBlockOffset() const { return GetReferenceBy(0x4002)->offset; }
u32 StreamSoundFile::FileHeader::GetRegionBlockOffset() const { return GetReferenceBy(0x4003)->offset; }
u32 StreamSoundFile::FileHeader::GetMarkerBlockOffset() const { return GetReferenceBy(0x4005)->offset; }

const StreamSoundFile::StreamSoundInfo* StreamSoundFile::InfoBlockBody::GetStreamSoundInfo() const {
    if (sound.type != 0x4100) return nullptr;
    return reinterpret_cast<const StreamSoundInfo*>(reinterpret_cast<const u8*>(this) + sound.offset);
}

const StreamSoundFile::TrackInfoTable* StreamSoundFile::InfoBlockBody::GetTrackInfoTable() const {
    if (tracks.type != 0x101) return nullptr;
    return reinterpret_cast<const TrackInfoTable*>(reinterpret_cast<const u8*>(this) + tracks.offset);
}

const StreamSoundFile::ChannelInfoTable* StreamSoundFile::InfoBlockBody::GetChannelInfoTable() const {
    if (channels.type != 0x101) return nullptr;
    return reinterpret_cast<const ChannelInfoTable*>(reinterpret_cast<const u8*>(this) + channels.offset);
}

// index selects a track; out-of-range indices and incorrect tags return null.
const StreamSoundFile::TrackInfo* StreamSoundFile::TrackInfoTable::GetTrackInfo(u32 index) const {
    if (index >= count) return nullptr;
    if (tracks[index].type != 0x4101) return nullptr;
    return reinterpret_cast<const TrackInfo*>(reinterpret_cast<const u8*>(this) + tracks[index].offset);
}

// index selects a channel; out-of-range indices and incorrect tags return null.
const StreamSoundFile::ChannelInfo* StreamSoundFile::ChannelInfoTable::GetChannelInfo(u32 index) const {
    if (index >= count) return nullptr;
    if (channels[index].type != 0x4102) return nullptr;
    return reinterpret_cast<const ChannelInfo*>(reinterpret_cast<const u8*>(this) + channels[index].offset);
}

const StreamSoundFile::DspAdpcmChannelInfo* StreamSoundFile::ChannelInfo::GetDspAdpcmChannelInfo() const {
    if (adpcm.type != 0x300) return nullptr;
    return reinterpret_cast<const DspAdpcmChannelInfo*>(reinterpret_cast<const u8*>(this) + adpcm.offset);
}
}
