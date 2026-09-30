#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>
#include <nn/atk/atk_DspAdpcmParam.h>

namespace nn::atk::detail {
struct StreamSoundFile {
    struct StreamSoundInfo {
        u8 sampleFormat, loop, channelCount, regionCount;
        u32 sampleRate, loopStart, loopEnd;
        u32 blockCount, blockSize, blockSampleCount;
        u32 lastBlockSize, lastBlockSampleCount, lastBlockPaddedSize;
        u32 seekInfoSize, seekIntervalSamples;
        Reference sampleData;
        u16 regionInfoSize, reserved;
        Reference regionData;
        u32 originalLoopStart, originalLoopEnd, crc32;
    };
    struct ChannelIndexTable { u32 count; u8 indices[1]; };
    struct TrackInfo {
        u8 volume, pan, surroundPan, _03;
        Reference channelIndices;
        const ChannelIndexTable* GetChannelIndices() const {
            return reinterpret_cast<const ChannelIndexTable*>(reinterpret_cast<const u8*>(this) + channelIndices.offset);
        }
    };
    struct DspAdpcmChannelInfo { DspAdpcmParam param; DspAdpcmLoopParam loop; };
    struct ChannelInfo {
        Reference adpcm;
        const DspAdpcmChannelInfo* GetDspAdpcmChannelInfo() const;
    };
    struct TrackInfoTable {
        u32 count;
        Reference tracks[1];
        const TrackInfo* GetTrackInfo(u32 index) const;
    };
    struct ChannelInfoTable {
        u32 count;
        Reference channels[1];
        const ChannelInfo* GetChannelInfo(u32 index) const;
    };
    struct InfoBlockBody {
        Reference sound, tracks, channels;
        const StreamSoundInfo* GetStreamSoundInfo() const;
        const TrackInfoTable* GetTrackInfoTable() const;
        const ChannelInfoTable* GetChannelInfoTable() const;
    };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        const ReferenceWithSize* GetReferenceBy(u16 type) const;
        bool HasSeekBlock() const;
        bool HasRegionBlock() const;
        bool HasMarkerBlock() const;
        u32 GetInfoBlockSize() const;
        u32 GetSeekBlockSize() const;
        u32 GetDataBlockSize() const;
        u32 GetRegionBlockSize() const;
        u32 GetMarkerBlockSize() const;
        u32 GetInfoBlockOffset() const;
        u32 GetSeekBlockOffset() const;
        u32 GetDataBlockOffset() const;
        u32 GetRegionBlockOffset() const;
        u32 GetMarkerBlockOffset() const;
    };
};
static_assert(sizeof(StreamSoundFile::StreamSoundInfo) == 0x50, "StreamSoundInfo size");
static_assert(sizeof(StreamSoundFile::DspAdpcmChannelInfo) == 0x2c, "DspAdpcmChannelInfo size");
}
