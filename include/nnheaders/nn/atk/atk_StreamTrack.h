#pragma once
#include <nn/types.h>

namespace nn::atk {
class WaveBuffer;
namespace detail::driver {
class MultiVoice {
public:
    void AppendWaveBuffer(int channel, WaveBuffer* buffer, bool last);
    void Pause(bool isPause);
};
class StreamChannel {
public:
    void AppendWaveBuffer(WaveBuffer* buffer, bool last);
private:
    u8 _0[8];
    MultiVoice* mVoice;
};
}
}
