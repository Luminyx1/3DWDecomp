#include <nn/atk/atk_WaveFile.h>

namespace nn::atk::detail {
const WaveFile::InfoBlock* WaveFile::FileHeader::GetInfoBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x7000) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const InfoBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}

const WaveFile::DataBlock* WaveFile::FileHeader::GetDataBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x7001) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const DataBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}

// index selects a channel reference relative to the channel-table header.
const WaveFile::ChannelInfo* WaveFile::InfoBlockBody::GetChannelInfo(int index) const {
    if (static_cast<unsigned>(index) >= channelCount) return nullptr;
    return reinterpret_cast<const ChannelInfo*>(reinterpret_cast<const u8*>(&channelCount) + channels[static_cast<unsigned>(index)].offset);
}

// data is the sample-data base to which this channel's sample offset is relative.
const void* WaveFile::ChannelInfo::GetSamplesAddress(const void* data) const {
    return static_cast<const u8*>(data) + samples.offset;
}

const WaveFile::DspAdpcmInfo* WaveFile::ChannelInfo::GetDspAdpcmInfo() const {
    return reinterpret_cast<const DspAdpcmInfo*>(reinterpret_cast<const u8*>(this) + adpcm.offset);
}
}
