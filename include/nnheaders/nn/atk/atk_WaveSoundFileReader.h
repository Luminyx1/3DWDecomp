#pragma once
#include <nn/atk/atk_WaveSoundFile.h>

namespace nn::atk::detail {
struct WaveSoundInfo {
    float pitch;
    AdshrCurve envelope;
    u8 pan, surroundPan, mainSend, auxSends[3];
    u8 lpfFrequency, biquadType, biquadValue;
};
struct WaveSoundNoteInfo {
    u32 archiveId, waveIndex;
    AdshrCurve envelope;
    u8 originalKey, pan, surroundPan, volume;
    float pitch;
};
static_assert(sizeof(WaveSoundInfo) == 0x14, "WaveSoundInfo size");
static_assert(sizeof(WaveSoundNoteInfo) == 0x18, "WaveSoundNoteInfo size");

class WaveSoundFileReader {
public:
    explicit WaveSoundFileReader(const void* file);
    u32 GetWaveSoundCount() const;
    u32 GetNoteInfoCount(u32 sound) const;
    u32 GetTrackInfoCount(u32 sound) const;
    bool ReadWaveSoundInfo(WaveSoundInfo* info, u32 sound) const;
    bool IsFilterSupportedVersion() const;
    bool ReadNoteInfo(WaveSoundNoteInfo* info, u32 sound, u32 note) const;
private:
    const WaveSoundFile::FileHeader* mHeader;
    const WaveSoundFile::InfoBlockBody* mInfo;
};
static_assert(sizeof(WaveSoundFileReader) == 0x10, "WaveSoundFileReader size");
}
