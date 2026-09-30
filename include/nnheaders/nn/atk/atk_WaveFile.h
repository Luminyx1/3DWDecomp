#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct WaveFile {
    struct InfoBlock;
    struct DataBlock;
    struct DspAdpcmInfo;
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
        u8 _00[0x14];
        u32 channelCount;
        Reference channels[1];
        const ChannelInfo* GetChannelInfo(int index) const;
    };
};
}
