#pragma once
#include <nn/atk/atk_AdvancedWaveSoundFile.h>

namespace nn::atk::detail {
struct AdvancedWaveSoundClipInfo {
    u32 waveIndex;
    u32 startTimeMilliseconds;
    u32 _08;
    u32 startOffsetMilliseconds;
    float pitch;
    u8 volume, pan;
};
struct AdvancedWaveSoundTrackInfo {
    int clipCount;
    AdvancedWaveSoundClipInfo clips[10];
};
struct AdvancedWaveSoundTrackInfoSet {
    int trackCount;
    AdvancedWaveSoundTrackInfo tracks[4];
};
static_assert(sizeof(AdvancedWaveSoundClipInfo) == 0x18, "AdvancedWaveSoundClipInfo size");
static_assert(sizeof(AdvancedWaveSoundTrackInfo) == 0xf4, "AdvancedWaveSoundTrackInfo size");
static_assert(sizeof(AdvancedWaveSoundTrackInfoSet) == 0x3d4, "AdvancedWaveSoundTrackInfoSet size");

class AdvancedWaveSoundFileReader {
public:
    explicit AdvancedWaveSoundFileReader(const void* file);
    int GetWaveSoundTrackCount() const;
    int GetWaveSoundClipCount(int track) const;
    bool ReadWaveSoundTrackInfoSet(AdvancedWaveSoundTrackInfoSet* info);
private:
    const AdvancedWaveSoundFile::InfoBlockBody* mInfo;
};
}
