#pragma once

#include <nn/types.h>

namespace nn::atk::detail {
struct WaveId {
    u32 waveArchiveId;
    u32 waveIndex;
};

struct WaveIdTable {
    u32 count;
    WaveId items[1];
};

struct VelocityRegionInfo {
    /** @brief Initializes the ADSHR parameters before reading a bank region. */
    VelocityRegionInfo() : attack(0), decay(0), sustain(0), hold(0), release(0) {}
    WaveId waveId;
    float pitch;
    u8 attack, decay, sustain, hold, release;
    u8 originalKey, volume, pan;
    u8 ignoreNoteOff, keyGroup, interpolationType;
};
static_assert(sizeof(VelocityRegionInfo) == 0x18, "VelocityRegionInfo size");

class BankFileReader {
  public:
    explicit BankFileReader(const void* pBankFile);

    const WaveIdTable* GetWaveIdTable() const;
    bool ReadVelocityRegionInfo(VelocityRegionInfo* pInfo, int program, int key, int velocity) const;

  private:
    u8 _0[0x18];
};
} // namespace nn::atk::detail
