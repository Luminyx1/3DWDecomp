#include <nn/atk/atk_AudioRendererPerformanceReader.h>
#include <nn/atk/atk_SoundSystem.h>
#include <cstring>

namespace nn::atk {
AudioRendererPerformanceReader::AudioRendererPerformanceReader()
    : mFrames(nullptr), mFrameCount(0), mWriteIndex(0), mReadIndex(0), mInitialized(false) {}

// frameCount is the number of ring slots, including the reserved slot separating read and write cursors.
size_t AudioRendererPerformanceReader::GetRequiredMemorySize(int frameCount) {
    return size_t(frameCount) * (SoundSystem::GetPerformanceFrameBufferSize() + sizeof(AudioRendererPerformanceInfo));
}

// frameCount sets the ring capacity; buffer holds descriptors followed by their frame buffers.
// bufferSize describes the supplied allocation; the original relies on the caller to provide enough space.
void AudioRendererPerformanceReader::Initialize(int frameCount, void* buffer, size_t bufferSize) {
    mFrameCount = frameCount;
    mFrames = static_cast<AudioRendererPerformanceInfo*>(buffer);
    size_t frameSize = SoundSystem::GetPerformanceFrameBufferSize();
    auto* data = static_cast<u8*>(buffer) + sizeof(AudioRendererPerformanceInfo) * frameCount;
    for (int i = 0; i < mFrameCount; ++i) {
        mFrames[i].buffer = data;
        mFrames[i].bufferSize = frameSize;
        data += frameSize;
    }

    mWriteIndex.store(0, std::memory_order_release);
    mReadIndex.store(mFrameCount - 1, std::memory_order_release);
    mInitialized = true;
}

const AudioRendererPerformanceInfo* AudioRendererPerformanceReader::ReadPerformanceInfo() {
    int next = mReadIndex.load(std::memory_order_acquire) + 1;
    if (next >= mFrameCount) next = 0;
    if (next == mWriteIndex.load(std::memory_order_acquire)) return nullptr;
    mReadIndex.store(next, std::memory_order_release);
    return &mFrames[next];
}

// frame supplies frameSize bytes to copy, and tick timestamps the captured frame.
// The caller must keep frameSize within a slot's capacity; a full ring drops the incoming frame.
void AudioRendererPerformanceReader::Record(const void* frame, size_t frameSize, os::Tick tick) {
    if (mWriteIndex.load(std::memory_order_acquire) == mReadIndex.load(std::memory_order_acquire)) return;
    int index = mWriteIndex.load(std::memory_order_acquire);
    mFrames[index].tick = tick;
    std::memcpy(mFrames[index].buffer, frame, frameSize);
    int next = index + 1;
    if (next >= mFrameCount) next = 0;
    mWriteIndex.store(next, std::memory_order_release);
}
}
