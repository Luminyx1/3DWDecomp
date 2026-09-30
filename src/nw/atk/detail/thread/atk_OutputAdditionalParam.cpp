#include <nn/atk/atk_OutputAdditionalParam.h>
#include <new>

namespace nn::atk::detail {
// config selects optional packets and the number of output buses needing storage.
size_t OutputAdditionalParam::GetRequiredMemSize(const SoundInstanceConfig& config) {
    size_t size = config.busCount > 4 ? sizeof(ValueArray<float>) + sizeof(float) * (config.busCount - 4) : 0;

    if (config.enableBusMixVolume)
        size += sizeof(BusMixVolumePacket) + BusMixVolumePacket::GetRequiredMemSize(config.busCount);
    if (config.enableVolumeThroughMode)
        size += sizeof(VolumeThroughModePacket) + VolumeThroughModePacket::GetRequiredMemSize(config.busCount);
    return size;
}

// memory and size describe caller-owned storage; config selects the packets constructed there.
// Capacity checks are absent in this build; disabled packet pointers must already be null.
void OutputAdditionalParam::Initialize(void* memory, size_t size, const SoundInstanceConfig& config) {
    auto* cursor = static_cast<u8*>(memory);

    if (config.busCount > 4) {
        mAdditionalSend = reinterpret_cast<ValueArray<float>*>(cursor);
        new (mAdditionalSend) ValueArray<float>;
        cursor += sizeof(ValueArray<float>);
        int count = config.busCount - 4;
        size_t bytes = count * sizeof(float);
        mAdditionalSend->Initialize(cursor, count);
        cursor += bytes;
    }

    if (config.enableBusMixVolume) {
        mBusMix = reinterpret_cast<BusMixVolumePacket*>(cursor);
        new (mBusMix) BusMixVolumePacket;
        cursor += sizeof(BusMixVolumePacket);
        size_t bytes = BusMixVolumePacket::GetRequiredMemSize(config.busCount);
        mBusMix->Initialize(cursor, bytes, config.busCount);
        cursor += bytes;
    }

    if (config.enableVolumeThroughMode) {
        mVolumeThrough = reinterpret_cast<VolumeThroughModePacket*>(cursor);
        new (mVolumeThrough) VolumeThroughModePacket;
        cursor += sizeof(VolumeThroughModePacket);
        size_t bytes = VolumeThroughModePacket::GetRequiredMemSize(config.busCount);
        mVolumeThrough->Initialize(cursor, bytes, config.busCount);
    }
}

void OutputAdditionalParam::Finalize() {
    if (mAdditionalSend) { mAdditionalSend->Finalize(); mAdditionalSend = nullptr; }
    if (mBusMix) { mBusMix = nullptr; }
    if (mVolumeThrough) { mVolumeThrough->Finalize(); mVolumeThrough = nullptr; }
}

void OutputAdditionalParam::Reset() {
    if (mAdditionalSend) mAdditionalSend->Reset();

    if (mBusMix) mBusMix->Reset();

    if (mVolumeThrough) mVolumeThrough->Reset();
}

void* OutputAdditionalParam::GetBufferAddr() {
    if (mAdditionalSend) return mAdditionalSend;

    if (mBusMix) return mBusMix;
    return mVolumeThrough;
}

ValueArray<float>* OutputAdditionalParam::GetAdditionalSendAddr() { return mAdditionalSend; }
const ValueArray<float>* OutputAdditionalParam::GetAdditionalSendAddr() const { return mAdditionalSend; }
// bus is an output bus index; indices beyond the four built-in buses access the extra sends.
float OutputAdditionalParam::TryGetAdditionalSend(int bus) const {
    int index = bus - 4;

    if (index < 0 || mAdditionalSend->mCount <= index) return 0.0f;
    return mAdditionalSend->mValues[index];
}

bool OutputAdditionalParam::IsAdditionalSendEnabled() const { return mAdditionalSend != nullptr; }
// bus selects an extra send and send is its gain; out-of-range indices are ignored.
void OutputAdditionalParam::TrySetAdditionalSend(int bus, float send) {
    int index = bus - 4;

    if (index < 0 || mAdditionalSend->mCount <= index) return;
    mAdditionalSend->mValues[index] = send;
}

BusMixVolumePacket* OutputAdditionalParam::GetBusMixVolumePacketAddr() { return mBusMix; }
const BusMixVolumePacket* OutputAdditionalParam::GetBusMixVolumePacketAddr() const { return mBusMix; }
// channel selects a mixing-table row and bus selects an entry within that row.
float OutputAdditionalParam::GetBusMixVolume(int channel, int bus) const { return mBusMix->mVolume.volumes[channel][bus]; }
const OutputBusMixVolume& OutputAdditionalParam::GetBusMixVolume() const { return mBusMix->mVolume; }
// channel and bus select the mixing entry; volume is its new gain.
void OutputAdditionalParam::SetBusMixVolume(int channel, int bus, float volume) { mBusMix->mVolume.volumes[channel][bus] = volume; }
// volume replaces the complete mixing table.
void OutputAdditionalParam::SetBusMixVolume(const OutputBusMixVolume& volume) { mBusMix->mVolume = volume; }
bool OutputAdditionalParam::IsBusMixVolumeUsed() const { return mBusMix->mUsed; }
// used selects whether the mixing table participates in output processing.
void OutputAdditionalParam::SetBusMixVolumeUsed(bool used) { mBusMix->SetUsed(used); }
// bus selects the enable flag to read.
bool OutputAdditionalParam::IsBusMixVolumeEnabledForBus(int bus) const { return mBusMix->mEnabled[bus]; }
// bus selects the enable flag and enabled supplies its new value.
void OutputAdditionalParam::SetBusMixVolumeEnabledForBus(int bus, bool enabled) { mBusMix->SetEnabledForBus(bus, enabled); }
bool OutputAdditionalParam::IsBusMixVolumeEnabled() const { return mBusMix != nullptr; }
VolumeThroughModePacket* OutputAdditionalParam::GetVolumeThroughModePacketAddr() { return mVolumeThrough; }
const VolumeThroughModePacket* OutputAdditionalParam::GetVolumeThroughModePacketAddr() const { return mVolumeThrough; }
float OutputAdditionalParam::GetBinaryVolume() const { return mVolumeThrough->mVolume; }
// volume is the binary's gain; writes are ignored when the optional packet is absent.
void OutputAdditionalParam::SetBinaryVolume(float volume) { if (mVolumeThrough) mVolumeThrough->mVolume = volume; }
// bus selects a mode byte; invalid indices return the default mode, zero.
u8 OutputAdditionalParam::TryGetVolumeThroughMode(int bus) const {
    if (bus < 0) return 0;

    if (bus >= mVolumeThrough->mBusCount) return 0;
    return mVolumeThrough->mModes[bus];
}

// bus selects the mode byte and mode supplies its flags; absent packets and invalid buses are ignored.
void OutputAdditionalParam::TrySetVolumeThroughMode(int bus, u8 mode) {
    if (bus < 0 || !mVolumeThrough || bus >= mVolumeThrough->mBusCount) return;
    mVolumeThrough->mModes[bus] = mode;
}

bool OutputAdditionalParam::IsVolumeThroughModeEnabled() const { return mVolumeThrough != nullptr; }
bool OutputAdditionalParam::IsVolumeThroughModeUsed() const { return mVolumeThrough && mVolumeThrough->mUsed; }
// used controls volume-through processing when its packet is present.
void OutputAdditionalParam::SetVolumeThroughModeUsed(bool used) { if (mVolumeThrough) mVolumeThrough->mUsed = used; }

// other supplies values for packets present in this object; storage ownership is retained.
OutputAdditionalParam& OutputAdditionalParam::operator=(const OutputAdditionalParam& other) {
    if (mAdditionalSend) {
        if (other.mAdditionalSend) *mAdditionalSend = *other.mAdditionalSend;
        else mAdditionalSend->Reset();
    }

    if (mBusMix && other.mBusMix) {
        mBusMix->mUsed = other.mBusMix->mUsed;

        if (mBusMix->mUsed) {
            mBusMix->mVolume = other.mBusMix->mVolume;
            int destinationCount = mBusMix->mBusCount;
            int sourceCount = other.mBusMix->mBusCount;
            int count = sourceCount < destinationCount ? sourceCount : destinationCount;

            for (int i = 0; i < count; ++i) mBusMix->mEnabled[i] = other.mBusMix->mEnabled[i];

            if (count < mBusMix->mBusCount) {
                // The original loop tests the bus count itself, rather than the index.
                for (int i = count; mBusMix->mBusCount; ++i) mBusMix->mEnabled[i] = false;
            }
        }
    }

    if (mVolumeThrough) {
        if (other.mVolumeThrough) *mVolumeThrough = *other.mVolumeThrough;
        else mVolumeThrough->Reset();
    }

    return *this;
}
}
