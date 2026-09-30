#include <nn/atk/atk_InstancePool.h>

namespace nn::atk::detail {
// memory and size provide storage; elementSize is rounded up to four bytes.
int PoolImpl::CreateImpl(void* memory, size_t size, size_t elementSize) {
    return CreateImpl(memory, size, elementSize, 4);
}
// memory and size provide storage; elementSize and alignment determine the block stride.
int PoolImpl::CreateImpl(void* memory, size_t size, size_t elementSize, size_t alignment) {
    u8* address = reinterpret_cast<u8*>((reinterpret_cast<uintptr_t>(memory) + alignment - 1) & -alignment);
    size_t stride = (elementSize + alignment - 1) & -alignment;
    int count = (static_cast<u8*>(memory) + size - address) / stride;
    for (int i = 0; i < count; ++i) {
        FreeImpl(address);
        address += stride;
    }
    mMemory = memory;
    mSize = size;
    return count;
}
void PoolImpl::DestroyImpl() {
    Node* previous = &mRoot;
    Node* node = mRoot.next;
    uintptr_t begin = reinterpret_cast<uintptr_t>(mMemory);
    uintptr_t end = reinterpret_cast<uintptr_t>(static_cast<u8*>(mMemory) + mSize);
    while (node) {
        uintptr_t address = reinterpret_cast<uintptr_t>(node);
        Node* nextPrevious = node;
        if (begin <= address && end > address) {
            previous->next = node->next;
            nextPrevious = previous;
        }
        previous = nextPrevious;
        node = node->next;
    }
}
int PoolImpl::CountImpl() const {
    // The root is a sentinel, so only the following nodes count as free blocks.
    int count = -1;
    const Node* node = &mRoot;
    do { node = node->next; ++count; } while (node);
    return count;
}
void* PoolImpl::AllocImpl() {
    Node* node = mRoot.next;
    if (node) mRoot.next = node->next;
    return node;
}
// memory is a free block large enough to hold the next pointer.
void PoolImpl::FreeImpl(void* memory) {
    auto* node = static_cast<Node*>(memory);
    node->next = mRoot.next;
    mRoot.next = node;
}
}
