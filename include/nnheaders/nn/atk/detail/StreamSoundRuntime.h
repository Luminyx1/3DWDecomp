/**
 * @file StreamSoundRuntime.h
 * @brief Stream sound runtime information.
 */

#pragma once

#include <nn/types.h>
#include <nn/atk/atk_LoaderManager.h>
#include <nn/atk/atk_StreamBufferPool.h>

namespace nn {
namespace atk {
namespace detail {
namespace driver {
class StreamSoundLoader;
}
class StreamSoundRuntime {
  public:
    StreamSoundRuntime();
    ~StreamSoundRuntime();

  private:
    void* mInstanceMemory;
    size_t mInstanceMemorySize;
    u8 _10[8];
    util::IntrusiveListNode mActiveSounds;
    util::IntrusiveListNode mFreeSounds;
    LoaderManager<driver::StreamSoundLoader> mLoaders;
    driver::StreamBufferPool mStreamBufferPool;
    driver::StreamBufferPool* mCurrentStreamBufferPool;
    int mStreamBufferTimes;
};
static_assert(sizeof(StreamSoundRuntime) == 0xb8, "StreamSoundRuntime size");
} // namespace detail
} // namespace atk
} // namespace nn
