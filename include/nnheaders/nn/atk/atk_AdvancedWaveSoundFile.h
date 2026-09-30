#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
struct AdvancedWaveSoundFile {
    struct ReferenceTable { int count; u32 offsets[1]; };
    struct WaveSoundClip;
    struct WaveSoundTrack {
        u32 _00, clipTableOffset;
        const ReferenceTable* GetClipReferenceTable() const;
        const WaveSoundClip* GetWaveSoundClip(int index) const;
    };
    struct InfoBlockBody {
        u32 trackTableOffset;
        const WaveSoundTrack* GetWaveSoundTrack(int index) const;
    };
};
}
