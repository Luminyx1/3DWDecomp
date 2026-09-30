#pragma once
#include <nn/os.h>
#include <atomic>

namespace nn::atk {
struct AudioRendererPerformanceInfo {
    void* buffer;
    size_t bufferSize;
    os::Tick tick;
};
class AudioRendererPerformanceReader {
public:
    AudioRendererPerformanceReader();
    static size_t GetRequiredMemorySize(int frameCount);
    void Initialize(int frameCount, void* buffer, size_t bufferSize);
    const AudioRendererPerformanceInfo* ReadPerformanceInfo();
    void Record(const void* frame, size_t frameSize, os::Tick tick);
private:
    AudioRendererPerformanceInfo* mFrames;
    int mFrameCount;
    std::atomic<int> mWriteIndex;
    std::atomic<int> mReadIndex;
    bool mInitialized;
};
static_assert(sizeof(AudioRendererPerformanceInfo) == 0x18, "Performance frame descriptor size");
static_assert(sizeof(AudioRendererPerformanceReader) == 0x18, "Performance reader size");
}
