#include <nn/atk/atk_SequenceSoundFile.h>
#include <nn/atk/atk_PlayerParamSet.h>
#include <cstring>

namespace nn::atk::detail {
const SequenceSoundFile::LabelBlock* SequenceSoundFile::FileHeader::GetLabelBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x5001) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const LabelBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }
    return nullptr;
}
// index selects a label reference; negative and out-of-range indices return null.
const SequenceSoundFile::LabelInfo* SequenceSoundFile::LabelBlockBody::GetLabelInfo(int index) const {
    if (count <= static_cast<u32>(index)) return nullptr;
    return reinterpret_cast<const LabelInfo*>(reinterpret_cast<const u8*>(this) + labels[static_cast<u32>(index)].offset);
}
// index must identify a valid label; the original accesses its text without a null check.
const char* SequenceSoundFile::LabelBlockBody::GetLabel(int index) const {
    return GetLabelInfo(index)->label;
}
// offset is the sequence-data position whose label is requested.
const char* SequenceSoundFile::LabelBlockBody::GetLabelByOffset(u32 offset) const {
    for (int i = 0; i < static_cast<int>(count); ++i) {
        auto* info = reinterpret_cast<const LabelInfo*>(reinterpret_cast<const u8*>(this) + labels[i].offset);
        if (info->offset == offset) return info->label;
    }
    return nullptr;
}
// index must be valid; offset receives the sequence-data position for that label.
bool SequenceSoundFile::LabelBlockBody::GetOffset(int index, u32* offset) const {
    auto* info = reinterpret_cast<const LabelInfo*>(reinterpret_cast<const u8*>(this) + labels[static_cast<u32>(index)].offset);
    *offset = info->offset;
    return true;
}
// label supplies the search prefix; offset receives the first matching label's position.
// The original compares only strlen(label) bytes, so a shorter prefix also matches.
bool SequenceSoundFile::LabelBlockBody::GetOffsetByLabel(const char* label, u32* offset) const {
    size_t length = std::strlen(label);
    for (int i = 0; i < static_cast<int>(count); ++i) {
        const LabelInfo* info = GetLabelInfo(i);
        if (std::strncmp(label, info->label, length) == 0) {
            *offset = info->offset;
            return true;
        }
    }
    return false;
}

namespace driver {
void PlayerParamSet::Initialize() {
    volume = 1.0f;
    pitch = 1.0f;
    _08 = 0.0f;
    biquadValue = 0.0f;
    biquadType = -1;
    _14 = 0;
    _18 = 0;
    _1c = 1;
    _24 = 0.0f;
    _20 = 1.0f;
    _58[0] = nullptr;
    _58[1] = nullptr;
    _58[2] = nullptr;
}
}
}
