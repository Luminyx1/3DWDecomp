#pragma once
#include <attributes.h>
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct SequenceSoundFile {
    struct DataBlock { u32 signature, size; u8 data[1]; };
    struct LabelBlock;
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        const LabelBlock* GetLabelBlock() const;
        // Keep the block lookup as a call, as in the original reader.
        NOINLINE const DataBlock* GetDataBlock() const {
            for (size_t i = 0; i < blockCount; ++i)
                if (blocks[i].type == 0x5000) {
                    s32 offset = blocks[i].offset;
                    return offset ? reinterpret_cast<const DataBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
                }
            return nullptr;
        }
    };
    struct LabelInfo {
        u32 _00;
        u32 offset;
        u32 length;
        char label[1];
    };
    struct LabelBlockBody {
        u32 count;
        Reference labels[1];
        const LabelInfo* GetLabelInfo(int index) const;
        const char* GetLabel(int index) const;
        const char* GetLabelByOffset(u32 offset) const;
        bool GetOffset(int index, u32* offset) const;
        bool GetOffsetByLabel(const char* label, u32* offset) const;
    };
    struct LabelBlock { u32 signature, size; LabelBlockBody body; };
};
}
