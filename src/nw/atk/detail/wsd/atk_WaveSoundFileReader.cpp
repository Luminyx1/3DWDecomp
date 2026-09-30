#include <nn/atk/atk_WaveSoundFileReader.h>

namespace nn::atk::detail {
// file points to a complete FWSD resource; unsupported headers leave the info pointer null.
WaveSoundFileReader::WaveSoundFileReader(const void* file) : mHeader(nullptr), mInfo(nullptr) {
    auto* header = static_cast<const WaveSoundFile::FileHeader*>(file);
    if (header->signature != 0x44535746 || header->byteOrder != 0xfeff ||
        header->version < 0x10000 || header->version > 0x10100) return;
    mHeader = header;
    const auto* info = mHeader->GetInfoBlock();
    if (!info || info->signature != 0x4f464e49) return;
    mInfo = &info->body;
}
u32 WaveSoundFileReader::GetWaveSoundCount() const { return mInfo->GetWaveSoundDataReferenceTable()->count; }
// sound selects a valid wave-sound record whose note count is requested.
u32 WaveSoundFileReader::GetNoteInfoCount(u32 sound) const {
    return mInfo->GetWaveSoundData(sound)->GetNoteInfoReferenceTable()->count;
}
// sound selects a valid wave-sound record whose track count is requested.
u32 WaveSoundFileReader::GetTrackInfoCount(u32 sound) const {
    return mInfo->GetWaveSoundData(sound)->GetTrackInfoReferenceTable()->count;
}
// info receives playback settings for sound; sound must be a valid record index.
bool WaveSoundFileReader::ReadWaveSoundInfo(WaveSoundInfo* info, u32 sound) const {
    const auto* source = mInfo->GetWaveSoundData(sound)->GetWaveSoundInfo();
    info->pitch = source->GetPitch();
    info->pan = source->GetPan();
    info->surroundPan = source->GetSurroundPan();
    source->GetSendValue(&info->mainSend, info->auxSends, 3);
    info->envelope = *source->GetAdshrCurve();
    if (IsFilterSupportedVersion()) {
        info->lpfFrequency = source->GetLpfFreq();
        info->biquadType = source->GetBiquadType();
        info->biquadValue = source->GetBiquadValue();
    } else {
        info->lpfFrequency = 64;
        info->biquadType = 0;
        info->biquadValue = 0;
    }
    return true;
}
bool WaveSoundFileReader::IsFilterSupportedVersion() const { return mHeader->version >= 0x10100; }
// info receives the note's wave ID and playback settings; sound and note select valid records.
// A note referring beyond the wave-ID table returns false without writing the output.
bool WaveSoundFileReader::ReadNoteInfo(WaveSoundNoteInfo* info, u32 sound, u32 note) const {
    const auto* source = mInfo->GetWaveSoundData(sound)->GetNoteInfo(note);
    const auto* table = mInfo->GetWaveIdTable();
    u32 index = source->waveIndex;
    if (table->count <= index) return false;
    info->archiveId = table->waves[index].archiveId;
    info->waveIndex = table->waves[index].waveIndex;
    info->pitch = source->GetPitch();
    info->envelope = *source->GetAdshrCurve();
    info->originalKey = source->GetOriginalKey();
    info->pan = source->GetPan();
    info->surroundPan = source->GetSurroundPan();
    info->volume = source->GetVolume();
    return true;
}
}
