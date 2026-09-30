#pragma once
#include <nn/atk/atk_DecodeAdpcm.h>

namespace nn::atk::detail {
struct AdpcmInfo {
    nn::audio::AdpcmParameter parameter;
    AdpcmContext context;
};
struct WaveInfo {
    struct ChannelInfo {
        const void* samples;
        u32 size;
        AdpcmInfo adpcm;
        AdpcmContext loopContext;
    };
    u32 sampleFormat;
    bool loop;
    int channelCount;
    int sampleRate;
    u64 loopStart;
    u64 loopEnd;
    u64 originalLoopStart;
    size_t dataSize;
    ChannelInfo channels[2];
};
static_assert(sizeof(AdpcmInfo) == 0x26, "AdpcmInfo size");
static_assert(sizeof(WaveInfo::ChannelInfo) == 0x38, "WaveInfo::ChannelInfo size");
static_assert(sizeof(WaveInfo) == 0xa0, "WaveInfo size");
}
