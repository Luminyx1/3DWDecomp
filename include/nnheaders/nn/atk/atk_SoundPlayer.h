#pragma once

#include <nn/types.h>

namespace nn::atk {
class SoundPlayer {
public:
    void StopAllSound(int fadeFrames);
    void PauseAllSound(bool flag, int fadeFrames);
};
}  // namespace nn::atk
