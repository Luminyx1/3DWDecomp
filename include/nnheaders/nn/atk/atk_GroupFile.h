#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct GroupFile {
    struct GroupItemInfo {
        u32 fileId;
        ReferenceWithSize file;
        // base is the FILE block body; an offset of -1 denotes an external file.
        const void* GetFileAddress(const void* base) const {
            s32 offset = file.offset;
            return offset == -1 ? nullptr : static_cast<const u8*>(base) + offset;
        }
    };
    struct GroupItemInfoEx { u32 itemId, loadFlags; };
    struct InfoBlockBody { u32 count; Reference items[1]; };
    struct InfoExBlockBody { u32 count; Reference items[1]; };
    struct InfoBlock { u32 signature, size; InfoBlockBody body; };
    struct FileBlock { u32 signature, size; u8 data[1]; };
    struct InfoExBlock { u32 signature, size; InfoExBlockBody body; };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        __attribute__((noinline)) const InfoBlock* GetInfoBlock() const;
        __attribute__((noinline)) const FileBlock* GetFileBlock() const;
        __attribute__((noinline)) const InfoExBlock* GetInfoExBlock() const;
    };
};
struct GroupItemLocationInfo {
    u32 fileId;
    const void* address;
};
}
