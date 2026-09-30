#include <nn/atk/atk_StreamBufferPool.h>
#include <cstring>

namespace nn::atk::detail::driver {
// memory and size describe DSP storage; count partitions it into aligned blocks.
void StreamBufferPool::Initialize(void* memory, size_t size, int count) {
    if (!count) return;
    Util::IsValidMemoryForDsp(memory, size);
    mMemory = static_cast<u8*>(memory);
    mSize = size;
    mBlockCount = count;
    mAllocatedCount = 0;
    std::memset(mAllocated, 0, sizeof(mAllocated));
    mBlockSize = (size / count) & ~size_t(63);
}

void StreamBufferPool::Finalize() {
    mBlockCount = 0;
    mBlockSize = 0;
    mSize = 0;
    mMemory = nullptr;
}

void* StreamBufferPool::Alloc() {
    if (mAllocatedCount >= mBlockCount) return nullptr;
    const int bytes = ((mBlockCount + 7) & ~7) / 8;
    for (int i = 0; i < bytes; ++i) {
        if (mAllocated[i] == 0xff) continue;
        for (int bit = 0; bit < 8; ++bit) {
            if (!(mAllocated[i] & (1 << bit))) {
                mAllocated[i] |= 1 << bit;
                ++mAllocatedCount;
                return mMemory + mBlockSize * (i * 8 + bit);
            }
        }
    }

    return nullptr;
}

// buffer is a currently allocated block returned by this pool.
void StreamBufferPool::Free(void* buffer) {
    size_t index = (static_cast<u8*>(buffer) - mMemory) / mBlockSize;
    mAllocated[index / 8] &= ~(1 << (index % 8));
    --mAllocatedCount;
}
}
