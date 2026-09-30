#pragma once
#include <nn/atk/atk_WaveFile.h>

namespace nn::atk::detail {
struct DspadpcmHeader {
    u32 sampleCount, nibbleCount, sampleRate;
    u16 loop, format;
    u32 loopStart, loopEnd, currentAddress;
    nn::audio::AdpcmParameter parameter;
    u16 gain;
    AdpcmContext context, loopContext;
    u8 reserved[0x16];
};
static_assert(sizeof(DspadpcmHeader) == 0x60, "DspadpcmHeader size");
class DspadpcmReader {
public:
    __attribute__((noinline)) DspadpcmReader();
    __attribute__((noinline)) bool ReadWaveInfo(WaveInfo* info) const;
private:
    friend class WaveFileReader;
    const DspadpcmHeader* mHeader;
};
class WaveFileReader {
public:
    WaveFileReader(const void* file, s8 type);
    // format is the file's encoding byte; unknown encodings use DSP ADPCM.
    static u32 GetSampleFormat(u8 format) { return format > 2 ? 2 : format; }
    bool IsOriginalLoopAvailable() const;
    bool ReadWaveInfo(WaveInfo* info, const void* waveData) const;
    const void* GetWaveDataAddress(const WaveFile::ChannelInfo* channel, const void* waveData) const;
private:
    const WaveFile::FileHeader* mHeader;
    const WaveFile::InfoBlockBody* mInfo;
    const void* mData;
    DspadpcmReader mDsp;
    s8 mType;
};
static_assert(sizeof(WaveFileReader) == 0x28, "WaveFileReader size");
}
