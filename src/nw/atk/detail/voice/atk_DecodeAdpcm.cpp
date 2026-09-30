#include <nn/atk/atk_DecodeAdpcm.h>

namespace nn::atk::detail {
// offset selects the first sample in data; context carries the predictor and history.
// parameter supplies predictor coefficients, count is the sample count, and output receives PCM16.
void DecodeDspAdpcm(long offset, AdpcmContext& context, const nn::audio::AdpcmParameter& parameter,
                    const void* data, size_t count, s16* output) {
    const u8* frame = static_cast<const u8*>(data) + (offset / 14) * 8;
    long sample = offset - (offset / 14) * 14;
    int predictor = context.predictorScale >> 4;
    int scale = context.predictorScale & 15;
    for (int i = 0; static_cast<unsigned>(i) < count; ++i) {
        if (sample == 0) {
            context.predictorScale = *frame;
            predictor = context.predictorScale >> 4;
            scale = context.predictorScale & 15;
        }
        int nibble = frame[1 + sample / 2];
        if (!(sample & 1)) nibble >>= 4;
        s16 previous = context.previousSample;
        int value = previous * parameter.coefficients[predictor * 2]
                  + context.previousSample2 * parameter.coefficients[predictor * 2 + 1]
                  + static_cast<s16>(1 << scale) * (static_cast<s16>(nibble << 12) >> 1);
        if (value > 0x3fffbff) value = 32767;
        else {
            value = (value >> 10) + 1;
            value = value < -65536 ? -32768 : value >> 1;
        }
        context.previousSample2 = previous;
        context.previousSample = value;
        output[i] = value;
        if (++sample == 14) { sample = 0; frame += 8; }
    }
}
}
