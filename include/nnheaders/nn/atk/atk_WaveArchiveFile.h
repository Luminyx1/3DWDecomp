#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct WaveArchiveFile {
    struct InfoBlockBody { u32 count; ReferenceWithSize waves[1]; };
    struct InfoBlock { u32 signature, size; InfoBlockBody body; };
    struct FileBlock { u32 signature, size; u8 data[1]; };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[2];
        const ReferenceWithSize* GetReferenceBy(u16 type) const;
        const InfoBlock* GetInfoBlock() const;
        u32 GetInfoBlockOffset() const;
        u32 GetInfoBlockSize() const;
        const FileBlock* GetFileBlock() const;
        u32 GetFileBlockOffset() const;
        u32 GetFileBlockSize() const;
    };
};
}
