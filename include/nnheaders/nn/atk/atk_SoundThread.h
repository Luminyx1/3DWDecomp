#pragma once

#include <nn/types.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk::detail::driver {
class SoundThread {
  public:
    class SoundFrameCallback {
      public:
        /** @brief Constructs an unregistered sound-frame callback. */
        SoundFrameCallback() = default;
        /** @brief Destroys the callback without changing its registration. */
        virtual ~SoundFrameCallback() = default;
        virtual void OnBeginSoundFrame() = 0;
        virtual void OnEndSoundFrame();

      private:
        util::IntrusiveListNode mNode;
    };
    typedef void (*SoundFrameUserCallback)(uintptr_t arg);

    static SoundThread& GetInstance();
    void ForceWakeup();

    void RegisterSoundFrameUserCallback(SoundFrameUserCallback callback, uintptr_t arg);
    void ClearSoundFrameUserCallback();
};
} // namespace nn::atk::detail::driver
