#include <nn/atk/atk_StreamTrack.h>

namespace nn::atk::detail::driver {
// buffer supplies the next audio samples; last marks the final buffer in the stream.
void StreamChannel::AppendWaveBuffer(WaveBuffer* buffer, bool last) {
    if (mVoice != nullptr) mVoice->AppendWaveBuffer(0, buffer, last);
}
}
