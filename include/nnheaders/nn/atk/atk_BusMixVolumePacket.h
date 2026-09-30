#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
struct OutputBusMixVolume {
    float volumes[2][24];
};

class BusMixVolumePacket {
public:
    static size_t GetRequiredMemSize(int busCount);
    bool Initialize(void* memory, size_t size, int busCount);
    void Finalize();
    void Reset();

private:
    OutputBusMixVolume mVolume;
    bool mUsed;
    bool* mEnabled;
    int mBusCount;
};
static_assert(sizeof(BusMixVolumePacket) == 0xd8, "BusMixVolumePacket size");
}
