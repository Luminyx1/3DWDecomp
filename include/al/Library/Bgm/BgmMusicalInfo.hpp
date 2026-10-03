#pragma once

#include <basis/seadTypes.h>

namespace al {
struct BgmRhythmInfo {
    f32 beat = 0.0f;
    s32 animId = 0;
};

static_assert(sizeof(BgmRhythmInfo) == 0x8);

struct BgmChordInfo {
    f32 beat = 0.0f;
    s32 root = 0;
    s32 chordNum;
    s32 scaleNum;
    s32* chord;
    s32* scale;
};

static_assert(sizeof(BgmChordInfo) == 0x20);

struct BgmMusicalInfo {
    BgmRhythmInfo** rhythmInfoList = nullptr;
    s32 rhythmInfoNum = 0;
    BgmChordInfo** chordInfoList = nullptr;
    s32 chordInfoNum = 0;
};

static_assert(sizeof(BgmMusicalInfo) == 0x20);
}  // namespace al
