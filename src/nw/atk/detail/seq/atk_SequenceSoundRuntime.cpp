#include <nn/atk/detail/StreamSoundRuntime.h>

namespace nn::atk::detail {
/** @brief Constructs an empty stream runtime with five stream-buffer intervals. */
StreamSoundRuntime::StreamSoundRuntime()
    : mInstanceMemory(nullptr), mInstanceMemorySize(0), mCurrentStreamBufferPool(nullptr),
      mStreamBufferTimes(5) {}

/** @brief Destroys the runtime after its owner finalizes its resources. */
StreamSoundRuntime::~StreamSoundRuntime() = default;
} // namespace nn::atk::detail
