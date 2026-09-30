#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct StreamSoundFile {
    struct StreamSoundInfo;
    struct TrackInfo;
    struct DspAdpcmChannelInfo;
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
}
