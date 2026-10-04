#pragma once

#include <aal/components/aalAudioFrameProcessMgr.h>

namespace al {
class GamePadSystem;

/**
 * @brief Holds the wave vibration players, updated from the audio frame (partially reconstructed).
 */
class WaveVibrationHolder : public aal::IAudioFrameProcess {
public:
    WaveVibrationHolder(const GamePadSystem* pGamePadSystem);

    void audioFrameProcess() override;

private:
    unsigned char _18[0xe8 - 0x18];
};

static_assert(sizeof(WaveVibrationHolder) == 0xe8);
}  // namespace al
