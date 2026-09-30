#pragma once
#include <nn/types.h>
#include <nn/util/util_BinaryFormat.h>

namespace nn::atk::detail {
struct AdvancedWaveSoundFile : nn::util::BinaryFileHeader {
    struct ReferenceTable { int count; u32 offsets[1]; };
    struct WaveSoundClip {
        u32 waveIndex;
        u32 startTimeMilliseconds;
        u32 _08;
        u32 startOffsetMilliseconds;
        float pitch;
        u8 volume, pan;
    };
    struct WaveSoundTrack {
        u32 _00, clipTableOffset;
        const ReferenceTable* GetClipReferenceTable() const;
        const WaveSoundClip* GetWaveSoundClip(int index) const;
    };
    struct InfoBlockBody {
        u32 trackTableOffset;
        __attribute__((noinline)) const ReferenceTable* GetTrackReferenceTable() const;
        const WaveSoundTrack* GetWaveSoundTrack(int index) const;
    };
    struct InfoBlock : nn::util::BinaryBlockHeader { InfoBlockBody body; };
    __attribute__((noinline)) const InfoBlock* GetBlock() const;
};
}
