#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
class VolumeThroughModePacket {
public:
    static size_t GetRequiredMemSize(int busCount);
    bool Initialize(void* memory, size_t size, int busCount);
    void Finalize();
    void Reset();
    VolumeThroughModePacket& operator=(const VolumeThroughModePacket& other);

private:
    u8* mModes;
    int mBusCount;
    u8 _0c[4];
    bool mUsed;
    float mVolume;
};
static_assert(sizeof(VolumeThroughModePacket) == 0x18, "VolumeThroughModePacket size");
}
