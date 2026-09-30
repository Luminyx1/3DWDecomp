#pragma once
#include <nn/atk/atk_SequenceSoundFile.h>

namespace nn::atk::detail {
class SequenceSoundFileReader {
public:
    explicit SequenceSoundFileReader(const void* file);
    const void* GetSequenceData() const;
    bool GetOffsetByLabel(const char* label, u32* offset) const;
    const char* GetLabelByOffset(u32 offset) const;

private:
    const SequenceSoundFile::FileHeader* mHeader;
    const void* mSequenceData;
    const SequenceSoundFile::LabelBlockBody* mLabels;
};
static_assert(sizeof(SequenceSoundFileReader) == 0x18, "SequenceSoundFileReader size");
}
