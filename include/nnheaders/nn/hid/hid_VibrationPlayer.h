#pragma once

#include <nn/types.h>

namespace nn::hid {
struct VibrationModulation {
    f32 amplitudeLow;
    f32 frequencyLow;
    f32 amplitudeHigh;
    f32 frequencyHigh;
};

// Pointer-only declarations: node/player storage layout is not reconstructed yet.
class VibrationNode {
public:
    void SetModulationTo(const VibrationNode* pDestination, const VibrationModulation& rModulation);
};
class VibrationPlayer : public VibrationNode {
public:
    void Load(const void* pData, size_t size);
    void Play();
    void Stop();
    void SetLoop(bool loop);
    bool IsPlaying() const;
    bool IsLoop() const;
};
class VibrationNodeConnection {
public:
    const VibrationNode* GetDestination() const;
    VibrationModulation GetModulation() const;
};
class VibrationMixer;
}  // namespace nn::hid
