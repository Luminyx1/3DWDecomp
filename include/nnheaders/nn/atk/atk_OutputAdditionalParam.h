#pragma once
#include <nn/atk/atk_BusMixVolumePacket.h>
#include <nn/atk/atk_ValueArray.h>
#include <nn/atk/atk_VolumeThroughModePacket.h>

namespace nn::atk::detail {
struct SoundInstanceConfig {
    bool enableBusMixVolume;
    bool enableVolumeThroughMode;
    int busCount;
};

class OutputAdditionalParam {
public:
    static size_t GetRequiredMemSize(const SoundInstanceConfig& config);
    void Initialize(void* memory, size_t size, const SoundInstanceConfig& config);
    void Finalize();
    void Reset();
    void* GetBufferAddr();
    ValueArray<float>* GetAdditionalSendAddr();
    const ValueArray<float>* GetAdditionalSendAddr() const;
    float TryGetAdditionalSend(int bus) const;
    bool IsAdditionalSendEnabled() const;
    void TrySetAdditionalSend(int bus, float send);
    BusMixVolumePacket* GetBusMixVolumePacketAddr();
    const BusMixVolumePacket* GetBusMixVolumePacketAddr() const;
    float GetBusMixVolume(int channel, int bus) const;
    const OutputBusMixVolume& GetBusMixVolume() const;
    void SetBusMixVolume(int channel, int bus, float volume);
    void SetBusMixVolume(const OutputBusMixVolume& volume);
    bool IsBusMixVolumeUsed() const;
    void SetBusMixVolumeUsed(bool used);
    bool IsBusMixVolumeEnabledForBus(int bus) const;
    void SetBusMixVolumeEnabledForBus(int bus, bool enabled);
    bool IsBusMixVolumeEnabled() const;
    VolumeThroughModePacket* GetVolumeThroughModePacketAddr();
    const VolumeThroughModePacket* GetVolumeThroughModePacketAddr() const;
    float GetBinaryVolume() const;
    void SetBinaryVolume(float volume);
    u8 TryGetVolumeThroughMode(int bus) const;
    void TrySetVolumeThroughMode(int bus, u8 mode);
    bool IsVolumeThroughModeEnabled() const;
    bool IsVolumeThroughModeUsed() const;
    void SetVolumeThroughModeUsed(bool used);
    OutputAdditionalParam& operator=(const OutputAdditionalParam& other);

private:
    ValueArray<float>* mAdditionalSend;
    BusMixVolumePacket* mBusMix;
    VolumeThroughModePacket* mVolumeThrough;
};
static_assert(sizeof(OutputAdditionalParam) == 0x18, "OutputAdditionalParam size");
}
