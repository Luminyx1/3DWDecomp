#include <nn/atk/atk_AdvancedWaveSoundFileReader.h>
#include <cstring>

namespace nn::atk::detail {
namespace {
// version contains the resource's little-endian micro, minor, and major fields.
u32 ReadPackedVersion(const nn::util::BinVersion& version) {
    u32 packed;
    std::memcpy(&packed, &version, sizeof(packed));
    return packed;
}
}

// file points to a complete BAWSD resource. Invalid headers leave the reader uninitialized.
AdvancedWaveSoundFileReader::AdvancedWaveSoundFileReader(const void* file) {
    const auto* header = static_cast<const AdvancedWaveSoundFile*>(file);

    if (!header->signature.IsValid("BAWSD   ") || header->_byteOrderMark != 0xfeff ||
        ReadPackedVersion(header->version) != 0x10000) return;
    const auto* block = header->GetBlock();

    if (!block) return;
    const char* signature = block->signature._str;

    if (signature[0] != 'I' || signature[1] != 'N' || signature[2] != 'F' || signature[3] != 'O') return;
    mInfo = &block->body;
}

int AdvancedWaveSoundFileReader::GetWaveSoundTrackCount() const {
    return mInfo->GetTrackReferenceTable()->count;
}

// track selects a track in the file; it must be a valid index.
int AdvancedWaveSoundFileReader::GetWaveSoundClipCount(int track) const {
    return mInfo->GetWaveSoundTrack(track)->GetClipReferenceTable()->count;
}

// info receives the decoded tracks and clips. The file must fit its four tracks and ten clips per track.
bool AdvancedWaveSoundFileReader::ReadWaveSoundTrackInfoSet(AdvancedWaveSoundTrackInfoSet* info) {
    info->trackCount = GetWaveSoundTrackCount();

    for (int i = 0; i < info->trackCount; ++i) {
        const auto* track = mInfo->GetWaveSoundTrack(i);
        auto& output = info->tracks[i];
        output.clipCount = track->GetClipReferenceTable()->count;

        for (int j = 0; j < output.clipCount; ++j) {
            const auto* clip = track->GetWaveSoundClip(j);
            auto& result = output.clips[j];
            result.waveIndex = clip->waveIndex;
            result.startTimeMilliseconds = clip->startTimeMilliseconds;
            result._08 = clip->_08;
            result.startOffsetMilliseconds = clip->startOffsetMilliseconds;
            result.pitch = clip->pitch;
            result.volume = clip->volume;
            result.pan = clip->pan;
        }
    }

    return true;
}

const AdvancedWaveSoundFile::InfoBlock* AdvancedWaveSoundFile::GetBlock() const {
    return static_cast<const InfoBlock*>(GetFirstBlock());
}

const AdvancedWaveSoundFile::ReferenceTable* AdvancedWaveSoundFile::InfoBlockBody::GetTrackReferenceTable() const {
    return reinterpret_cast<const ReferenceTable*>(reinterpret_cast<const u8*>(this) + trackTableOffset);
}
}
