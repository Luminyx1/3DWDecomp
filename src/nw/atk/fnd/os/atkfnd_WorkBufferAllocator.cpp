#include <nn/atk/atkfnd_WorkBufferAllocator.h>

namespace nn::atk::detail::fnd {
// buffer supplies the arena; size is its capacity in bytes.
WorkBufferAllocator::WorkBufferAllocator(void* buffer, size_t size)
    : mBuffer(reinterpret_cast<uintptr_t>(buffer)), mOffset(0), mSize(size) {}

// buffer and size describe the arena; alignment is unused by this constructor.
WorkBufferAllocator::WorkBufferAllocator(void* buffer, size_t size, size_t alignment)
    : mBuffer(reinterpret_cast<uintptr_t>(buffer)), mOffset(0), mSize(size) {}

// size is the requested byte count; zero or insufficient capacity returns null.
void* WorkBufferAllocator::Allocate(size_t size) {
    if (!size) return nullptr;
    uintptr_t start = mBuffer + mOffset;
    uintptr_t end = start + size;
    if (end > mBuffer + mSize) return nullptr;
    mOffset = end - mBuffer;
    return reinterpret_cast<void*>(start);
}

// size requests bytes; alignment is the required power-of-two address alignment.
void* WorkBufferAllocator::Allocate(size_t size, size_t alignment) {
    void* result = nullptr;
    if (size) {
        uintptr_t start = (mBuffer + mOffset + alignment - 1) & -alignment;
        uintptr_t end = start + size;
        if (end <= mBuffer + mSize) {
            mOffset = end - mBuffer;
            result = reinterpret_cast<void*>(start);
        }
    }
    return result;
}

// size and alignment describe each block; count requests consecutive allocations.
// The original returns the first result even if a later allocation fails.
void* WorkBufferAllocator::Allocate(size_t size, size_t alignment, int count) {
    void* first = Allocate(size, alignment);
    for (int i = 1; i < count; ++i) Allocate(size, alignment);
    return first;
}
}
