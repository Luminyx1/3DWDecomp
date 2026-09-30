#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>
#include <nn/atk/atk_WaveInfo.h>

namespace nn::atk::detail {
struct WaveFile {
    struct InfoBlock;
    struct DataBlock;
    struct DspAdpcmInfo { AdpcmInfo adpcm; AdpcmContext loopContext; };
    struct ChannelInfo {
        Reference samples, adpcm;
        const void* GetSamplesAddress(const void* data) const;
        const DspAdpcmInfo* GetDspAdpcmInfo() const;
    };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        const InfoBlock* GetInfoBlock() const;
        const DataBlock* GetDataBlock() const;
    };
    struct InfoBlockBody {
        u8 sampleFormat, loop;
        u16 reserved;
        u32 sampleRate, loopStart, loopEnd, originalLoopStart;
        u32 channelCount;
        Reference channels[1];
        const ChannelInfo* GetChannelInfo(int index) const;
    };
    struct InfoBlock { u32 signature, size; InfoBlockBody body; };
    struct DataBlock { u32 signature, size; u8 data[1]; };
};
}
