#pragma once

#include <nn/types.h>

namespace nn::atk {

/** @brief Pause policy shared by sound players and individual sounds. */
enum PauseMode {};
enum OutputMode {
    OutputMode_Monaural,
    OutputMode_Stereo,
    OutputMode_Surround,
    OutputMode_Dpl2,
    OutputMode_Count
};

enum AuxBus {
    AuxBus_A,
    AuxBus_B,
    AuxBus_C,
    AuxBus_Count
};

enum OutputDevice {
    OutputDevice_Main,
    OutputDevice_Count
};

enum ChannelIndex {
    ChannelIndex_FrontLeft = 0,
    ChannelIndex_FrontRight = 1,
    ChannelIndex_RearLeft = 2,
    ChannelIndex_RearRight = 3,
    ChannelIndex_FrontCenter,
    ChannelIndex_Lfe,
    ChannelIndex_Count
};

namespace detail::Util {
template <typename T>
class Singleton {
public:
    static T& GetInstance();
};
}  // namespace detail::Util
}  // namespace nn::atk
