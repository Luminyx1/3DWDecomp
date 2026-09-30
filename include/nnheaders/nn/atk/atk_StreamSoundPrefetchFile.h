#pragma once
#include <nn/atk/atk_StreamSoundFile.h>

namespace nn::atk::detail {
struct StreamSoundPrefetchFile {
    struct PrefetchSample {
        u8 samples[1];
        const void* GetSampleAddress() const;
    };
    struct PrefetchData {
        u32 startFrame, sampleBytes, _08;
        Reference sample;
        const PrefetchSample* GetPrefetchSample() const;
    };
    struct PrefetchDataBlockBody { u32 count; PrefetchData entries[1]; };
    struct InfoBlock { u32 signature, size; StreamSoundFile::InfoBlockBody body; };
    struct RegionBlock { u32 signature, size; };
    struct PrefetchDataBlock { u32 signature, size; PrefetchDataBlockBody body; };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        const InfoBlock* GetInfoBlock() const;
        const RegionBlock* GetRegionBlock() const;
        const PrefetchDataBlock* GetPrefetchDataBlock() const;
        u32 GetPrefetchDataBlockSize() const;
        bool HasRegionBlock() const;
        u32 GetRegionBlockSize() const;
        u32 GetRegionBlockOffset() const;
    };
};
static_assert(sizeof(StreamSoundPrefetchFile::PrefetchData) == 0x14, "PrefetchData size");
}
