#pragma once
#include <nn/types.h>

namespace nn::atk::detail::fnd {
class WorkBufferAllocator {
public:
    WorkBufferAllocator(void* buffer, size_t size);
    WorkBufferAllocator(void* buffer, size_t size, size_t alignment);
    void* Allocate(size_t size);
    void* Allocate(size_t size, size_t alignment);
    void* Allocate(size_t size, size_t alignment, int count);

private:
    uintptr_t mBuffer;
    size_t mOffset;
    size_t mSize;
};
static_assert(sizeof(WorkBufferAllocator) == 0x18, "WorkBufferAllocator size");
}
