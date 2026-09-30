#pragma once

#include <nn/types.h>

namespace nn::atk::detail::driver {
class SoundThread {
public:
    typedef void (*SoundFrameUserCallback)(uintptr_t arg);

    static SoundThread& GetInstance();
    void ForceWakeup();

    void RegisterSoundFrameUserCallback(SoundFrameUserCallback callback, uintptr_t arg);
    void ClearSoundFrameUserCallback();
};
}  // namespace nn::atk::detail::driver
