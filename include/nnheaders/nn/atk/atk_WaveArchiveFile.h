#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct WaveArchiveFile {
    struct InfoBlock;
    struct FileBlock;
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
