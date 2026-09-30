#include <nn/atk/atk_VolumeThroughModePacket.h>

namespace nn::atk::detail {
// busCount is the number of mode bytes; storage is aligned to eight bytes.
size_t VolumeThroughModePacket::GetRequiredMemSize(int busCount) {
    return (static_cast<size_t>(busCount) + 7) & ~size_t(7);
}

// memory holds busCount mode bytes; size is its capacity, unchecked in this build.
bool VolumeThroughModePacket::Initialize(void* memory, size_t size, int busCount) {
    mModes = static_cast<u8*>(memory);
    mBusCount = busCount;

    for (int i = 0; i < mBusCount; ++i) mModes[i] = 0;
    return true;
}

void VolumeThroughModePacket::Finalize() {
    mModes = nullptr;
    mBusCount = 0;
    mUsed = false;
    mVolume = 1.0f;
}

void VolumeThroughModePacket::Reset() {
    for (int i = 0; i < mBusCount; ++i) mModes[i] = 0;
    mUsed = false;
    mVolume = 1.0f;
}

// other supplies mode bytes and volume state; excess destination entries are cleared.
VolumeThroughModePacket& VolumeThroughModePacket::operator=(const VolumeThroughModePacket& other) {
    int count = other.mBusCount < mBusCount ? other.mBusCount : mBusCount;

    for (int i = 0; i < count; ++i) mModes[i] = other.mModes[i];

    for (int i = count; i < mBusCount; ++i) mModes[i] = 0;
    mUsed = other.mUsed;
    mVolume = other.mVolume;
    return *this;
}
}
