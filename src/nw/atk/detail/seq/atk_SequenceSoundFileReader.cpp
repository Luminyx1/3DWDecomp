#include <nn/atk/atk_SequenceSoundFileReader.h>

namespace nn::atk::detail {
// file points to the complete sequence file; unsupported headers leave data pointers null.
SequenceSoundFileReader::SequenceSoundFileReader(const void* file)
    : mHeader(nullptr), mSequenceData(nullptr), mLabels(nullptr) {
    auto* header = static_cast<const SequenceSoundFile::FileHeader*>(file);
    if (header->signature != 0x51455346 || header->byteOrder != 0xfeff ||
        header->version < 0x10000 || header->version > 0x20000) return;
    mHeader = header;
    const auto* data = mHeader->GetDataBlock();
    if (data->signature != 0x41544144) return;
    const auto* labels = mHeader->GetLabelBlock();
    if (labels->signature != 0x4c42414c) return;
    mSequenceData = data->data;
    mLabels = &labels->body;
}

const void* SequenceSoundFileReader::GetSequenceData() const { return mSequenceData; }
// label is the search prefix; offset receives the matching sequence-data position.
bool SequenceSoundFileReader::GetOffsetByLabel(const char* label, u32* offset) const {
    return mLabels->GetOffsetByLabel(label, offset);
}

// offset is the sequence-data position whose label is requested.
const char* SequenceSoundFileReader::GetLabelByOffset(u32 offset) const {
    return mLabels->GetLabelByOffset(offset);
}
}
