#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
namespace Util { bool IsValidMemoryForDsp(const void* memory, size_t size); }
namespace driver {
class StreamBufferPool {
public:
    void Initialize(void* memory, size_t size, int count);
    void Finalize();
    void* Alloc();
    void Free(void* buffer);

private:
    u8* mMemory;
    size_t mSize;
    size_t mBlockSize;
    int mBlockCount;
    int mAllocatedCount;
    u8 mAllocated[4];
};
static_assert(sizeof(StreamBufferPool) == 0x28, "StreamBufferPool size");
}
}
