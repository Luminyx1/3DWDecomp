#include <nn/atk/atk_AdvancedWaveSoundFile.h>

namespace nn::atk::detail {
// index selects the track in the info block's relative-offset table.
const AdvancedWaveSoundFile::WaveSoundTrack* AdvancedWaveSoundFile::InfoBlockBody::GetWaveSoundTrack(int index) const {
    const auto* table = reinterpret_cast<const ReferenceTable*>(reinterpret_cast<const u8*>(this) + trackTableOffset);
    if (index >= table->count) return nullptr;
    return reinterpret_cast<const WaveSoundTrack*>(reinterpret_cast<const u8*>(table) + table->offsets[index]);
}
const AdvancedWaveSoundFile::ReferenceTable* AdvancedWaveSoundFile::WaveSoundTrack::GetClipReferenceTable() const {
    return reinterpret_cast<const ReferenceTable*>(reinterpret_cast<const u8*>(this) + clipTableOffset);
}
// index selects the clip in this track's relative-offset table.
const AdvancedWaveSoundFile::WaveSoundClip* AdvancedWaveSoundFile::WaveSoundTrack::GetWaveSoundClip(int index) const {
    const auto* table = GetClipReferenceTable();
    if (index >= table->count) return nullptr;
    return reinterpret_cast<const WaveSoundClip*>(reinterpret_cast<const u8*>(table) + table->offsets[index]);
}
}
