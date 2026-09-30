#include <nn/atk/atk_BusMixVolumePacket.h>

namespace nn::atk::detail {
// busCount is the number of per-bus enable flags; storage is aligned to eight bytes.
size_t BusMixVolumePacket::GetRequiredMemSize(int busCount) {
    return (static_cast<size_t>(busCount) + 7) & ~size_t(7);
}

// memory holds busCount enable flags; size is its capacity, unchecked in this build.
bool BusMixVolumePacket::Initialize(void* memory, size_t size, int busCount) {
    mEnabled = static_cast<bool*>(memory);

    for (int i = 0; i < busCount; ++i) mEnabled[i] = false;
    mBusCount = busCount;
    return true;
}

void BusMixVolumePacket::Finalize() {
    mEnabled = nullptr;
    mUsed = false;
    mBusCount = 0;
}

void BusMixVolumePacket::Reset() {
    for (auto& output : mVolume.volumes)
        for (float& volume : output) volume = 1.0f;
    mUsed = false;

    for (int i = 0; i < mBusCount; ++i) mEnabled[i] = false;
}
}
