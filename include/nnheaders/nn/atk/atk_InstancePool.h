#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
class PoolImpl {
public:
    int CreateImpl(void* memory, size_t size, size_t elementSize);
    int CreateImpl(void* memory, size_t size, size_t elementSize, size_t alignment);
    void DestroyImpl();
    int CountImpl() const;
    void* AllocImpl();
    void FreeImpl(void* memory);
private:
    struct Node { Node* next; };
    Node mRoot;
    void* mMemory;
    size_t mSize;
};
static_assert(sizeof(PoolImpl) == 0x18, "PoolImpl size");
}
