#include <nn/atk/atk_WaveSoundFile.h>
#include <cstring>

namespace nn::atk::detail {
namespace {
const AdshrCurve DefaultAdshrCurve = {127, 127, 127, 127, 127};

// base is the origin of the signed byte offset to the requested record type.
template <typename T>
const T* AtOffset(const void* base, s32 offset) {
    return reinterpret_cast<const T*>(static_cast<const u8*>(base) + offset);
}

// table contains typed references; index selects one and type is the expected tag.
template <typename T>
const T* GetRecord(const WaveSoundFile::ReferenceTable* table, u32 index, u16 type) {
    if (index >= table->count) return nullptr;

    if (table->items[index].type != type) return nullptr;
    return AtOffset<T>(table, table->items[index].offset);
}

// flags identifies the stored parameters; bit selects the desired optional value.
// The returned word index includes the flags word; zero denotes an absent value.
u32 GetParameterIndex(u32 flags, unsigned bit) {
    if (!(flags & (1u << bit))) return 0;
    u32 index = 0;

    for (unsigned i = 0; i < bit; ++i)
        if (flags & (1u << i)) ++index;
    return index + 1;
}

// flags points to the flags word and its packed values; bit selects a present value.
u32 GetParameter(const u32* flags, unsigned bit) {
    size_t index = 0;

    for (unsigned i = 0; i < bit; ++i) index += (*flags >> i) & 1;
    return flags[index + 1];
}

// flags points to packed values; bit selects a present floating-point parameter.
float GetFloatParameter(const u32* flags, unsigned bit) {
    u32 bits = GetParameter(flags, bit);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
}

const WaveSoundFile::InfoBlock* WaveSoundFile::FileHeader::GetInfoBlock() const {
    for (size_t i = 0; i < blockCount; ++i)
        if (blocks[i].type == 0x6800) {
            s32 offset = blocks[i].offset;
            return offset ? reinterpret_cast<const InfoBlock*>(reinterpret_cast<const u8*>(this) + offset) : nullptr;
        }

    return nullptr;
}

// index selects a sound; missing entries and incorrect reference tags return null.
const WaveSoundFile::WaveSoundData* WaveSoundFile::InfoBlockBody::GetWaveSoundData(u32 index) const {
    return GetRecord<WaveSoundData>(GetWaveSoundDataReferenceTable(), index, 0x4900);
}

const WaveSoundFile::ReferenceTable* WaveSoundFile::InfoBlockBody::GetWaveSoundDataReferenceTable() const {
    return AtOffset<ReferenceTable>(this, sounds.offset);
}

const WaveSoundFile::WaveIdTable* WaveSoundFile::InfoBlockBody::GetWaveIdTable() const {
    return AtOffset<WaveIdTable>(this, waveIds.offset);
}

const WaveSoundFile::WaveSoundInfo* WaveSoundFile::WaveSoundData::GetWaveSoundInfo() const {
    return AtOffset<WaveSoundInfo>(this, info.offset);
}

const WaveSoundFile::ReferenceTable* WaveSoundFile::WaveSoundData::GetTrackInfoReferenceTable() const {
    return AtOffset<ReferenceTable>(this, tracks.offset);
}

const WaveSoundFile::ReferenceTable* WaveSoundFile::WaveSoundData::GetNoteInfoReferenceTable() const {
    return AtOffset<ReferenceTable>(this, notes.offset);
}

// index selects a track; missing entries and incorrect reference tags return null.
const WaveSoundFile::TrackInfo* WaveSoundFile::WaveSoundData::GetTrackInfo(u32 index) const {
    return GetRecord<TrackInfo>(GetTrackInfoReferenceTable(), index, 0x4903);
}

// index selects a note; missing entries and incorrect reference tags return null.
const WaveSoundFile::NoteInfo* WaveSoundFile::WaveSoundData::GetNoteInfo(u32 index) const {
    return GetRecord<NoteInfo>(GetNoteInfoReferenceTable(), index, 0x4902);
}

u8 WaveSoundFile::WaveSoundInfo::GetPan() const { return flags & 1 ? GetParameter(&flags, 0) : 64; }
u8 WaveSoundFile::WaveSoundInfo::GetSurroundPan() const { return flags & 1 ? GetParameter(&flags, 0) >> 8 : 0; }
float WaveSoundFile::WaveSoundInfo::GetPitch() const { return flags & 2 ? GetFloatParameter(&flags, 1) : 1.0f; }
// mainSend receives the main-bus level; auxSends receives up to three stored bus levels.
// auxCount is the number of auxiliary outputs to clear when the parameter is absent.
void WaveSoundFile::WaveSoundInfo::GetSendValue(u8* mainSend, u8* auxSends, u8 auxCount) const {
    u32 index = GetParameterIndex(flags, 8);

    if (!index) {
        *mainSend = 127;
        for (int i = 0; i < auxCount; ++i) auxSends[i] = 0;
        return;
    }

    const u8* send = reinterpret_cast<const u8*>(this) + (&flags)[index];
    *mainSend = send[0];
    unsigned count = send[1] < 3 ? send[1] : 3;

    for (unsigned i = 0; i < count; ++i) auxSends[i] = send[2 + i];
}

const AdshrCurve* WaveSoundFile::WaveSoundInfo::GetAdshrCurve() const {
    u32 index = GetParameterIndex(flags, 9);

    if (!index) return &DefaultAdshrCurve;
    const auto* ref = reinterpret_cast<const Reference*>(reinterpret_cast<const u8*>(this) + (&flags)[index]);
    return AtOffset<AdshrCurve>(ref, ref->offset);
}

u8 WaveSoundFile::WaveSoundInfo::GetLpfFreq() const { return flags & 4 ? GetParameter(&flags, 2) : 64; }
u8 WaveSoundFile::WaveSoundInfo::GetBiquadType() const { return flags & 4 ? GetParameter(&flags, 2) >> 8 : 0; }
u8 WaveSoundFile::WaveSoundInfo::GetBiquadValue() const { return flags & 4 ? GetParameter(&flags, 2) >> 16 : 0; }
const WaveSoundFile::ReferenceTable* WaveSoundFile::TrackInfo::GetNoteEventReferenceTable() const {
    return AtOffset<ReferenceTable>(this, events.offset);
}

// index selects an event; missing entries and incorrect reference tags return null.
const WaveSoundFile::NoteEvent* WaveSoundFile::TrackInfo::GetNoteEvent(u32 index) const {
    return GetRecord<NoteEvent>(GetNoteEventReferenceTable(), index, 0x4904);
}

u8 WaveSoundFile::NoteInfo::GetOriginalKey() const { return flags & 1 ? GetParameter(&flags, 0) : 64; }
u8 WaveSoundFile::NoteInfo::GetVolume() const { return flags & 2 ? GetParameter(&flags, 1) : 96; }
u8 WaveSoundFile::NoteInfo::GetPan() const { return flags & 4 ? GetParameter(&flags, 2) : 64; }
u8 WaveSoundFile::NoteInfo::GetSurroundPan() const { return flags & 4 ? GetParameter(&flags, 2) >> 8 : 0; }
float WaveSoundFile::NoteInfo::GetPitch() const { return flags & 8 ? GetFloatParameter(&flags, 3) : 1.0f; }
// mainSend receives the main-bus level; auxSends points to up to three stored bus outputs.
// auxCount is the number of pointed-to auxiliary outputs to clear if the parameter is absent.
void WaveSoundFile::NoteInfo::GetSendValue(u8* mainSend, u8** auxSends, u8 auxCount) const {
    u32 index = GetParameterIndex(flags, 8);

    if (index) {
        const u8* send = reinterpret_cast<const u8*>(this) + (&flags)[index];
        *mainSend = send[0];
        unsigned count = send[1] < 3 ? send[1] : 3;

        for (unsigned i = 0; i < count; ++i) *auxSends[i] = send[2 + i];
    } else {
        *mainSend = 127;
        for (int i = 0; i < auxCount; ++i) *auxSends[i] = 0;
    }
}

const AdshrCurve* WaveSoundFile::NoteInfo::GetAdshrCurve() const {
    u32 index = GetParameterIndex(flags, 9);

    if (!index) return &DefaultAdshrCurve;
    const auto* ref = reinterpret_cast<const Reference*>(reinterpret_cast<const u8*>(this) + (&flags)[index]);
    return AtOffset<AdshrCurve>(ref, ref->offset);
}
}
