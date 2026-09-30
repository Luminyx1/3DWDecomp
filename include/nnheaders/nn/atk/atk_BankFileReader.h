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

class BankFileReader {
public:
    explicit BankFileReader(const void* pBankFile);

    const WaveIdTable* GetWaveIdTable() const;

private:
    u8 _0[0x18];
};
}  // namespace nn::atk::detail
