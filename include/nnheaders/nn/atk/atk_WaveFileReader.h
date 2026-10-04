#pragma once
#include <attributes.h>
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
    NOINLINE DspadpcmReader();
    NOINLINE bool ReadWaveInfo(WaveInfo* info) const;

  private:
    friend class WaveFileReader;
    const DspadpcmHeader* mHeader;
};
class WaveFileReader {
  public:
    WaveFileReader(const void* file, s8 type);
    /**
     * @brief Converts the wave file's encoding byte to a playback sample format.
     * @param format File encoding value; unknown encodings select DSP ADPCM.
     * @return Recognized encoding value, or the DSP ADPCM format value of 2.
     */
    USED static u32 GetSampleFormat(u8 format) {
        switch (format) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        default:
            return 2;
        }
    }
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
} // namespace nn::atk::detail
