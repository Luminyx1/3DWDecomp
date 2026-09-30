#include <nn/atk/atk_ChannelMixVolume.h>

namespace nn::atk {
ChannelMixVolume::ChannelMixVolume() : mChannelCount(6) { InitializeChannelVolume(); }
void ChannelMixVolume::InitializeChannelVolume() {}

// volume supplies the six surround-channel gains in speaker order.
ChannelMixVolume::ChannelMixVolume(const MixVolume& volume) : mChannelCount(6) {
    mVolumes[0] = volume.frontLeft;
    mVolumes[1] = volume.frontRight;
    mVolumes[2] = volume.rearLeft;
    mVolumes[3] = volume.rearRight;
    mVolumes[4] = volume.frontCenter;
    mVolumes[5] = volume.lfe;
}

// volumes supplies count gains; an invalid count retains the six-channel default.
ChannelMixVolume::ChannelMixVolume(const float* volumes, int count) {
    if (static_cast<unsigned>(count) - 1 > 23) { mChannelCount = 6; return; }
    mChannelCount = count;
    for (int i = 0; i < count; ++i) mVolumes[i] = volumes[i];
}

// count selects between one and 24 active channels without changing their gains.
bool ChannelMixVolume::SetChannelCount(int count) {
    if (count < 1 || count > 24) return false;
    mChannelCount = count;
    return true;
}

// index selects an active channel; volume is its new gain.
bool ChannelMixVolume::SetChannelVolume(int index, float volume) {
    if (index < 0 || index >= mChannelCount) return false;
    mVolumes[index] = volume;
    return true;
}

// index selects an active channel; invalid indices return zero.
float ChannelMixVolume::GetChannelVolume(int index) const {
    if (index < 0 || index >= mChannelCount) return 0;
    return mVolumes[index];
}

// index is the first destination; volumes supplies count consecutive gains.
// The range comparison below preserves the original function's inverted check.
bool ChannelMixVolume::SetChannelVolume(int index, const float* volumes, int count) {
    if (index < 0) return false;
    const int last = static_cast<unsigned>(index) + count - 1;
    if (last < 24) return false;
    for (int i = 0; i < count; ++i) mVolumes[index + i] = volumes[i];
    return true;
}
}
