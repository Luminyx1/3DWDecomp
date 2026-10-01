#include "Project/Effect/EffectHeap.hpp"

#include "Library/Memory/Util.hpp"

namespace al {

/**
 * Creates an effect heap inside a newly allocated buffer of the current heap.
 * @param size Size of the buffer.
 * @param pName Name of the heap.
 * @return The created heap.
 */
EffectHeap* EffectHeap::create(u32 size, const char* pName) {
    u8* buffer = new u8[size];
    auto* heap = new (buffer) EffectHeap(pName, buffer, size);
    doCreate(heap, getCurrentHeap());
    return heap;
}

/**
 * Constructs an effect heap over a buffer, parented to the current heap.
 * @param pName Name of the heap.
 * @param pAddress Start of the buffer.
 * @param size Size of the buffer.
 */
EffectHeap::EffectHeap(const char* pName, void* pAddress, u32 size)
    : sead::ExpHeap(pName, getCurrentHeap(), pAddress, size, cHeapDirection_Forward, true) {}

/**
 * Allocates memory only if a large enough free block exists.
 * @param size Size of the allocation.
 * @param alignment Alignment of the allocation.
 * @return The allocated memory, or nullptr.
 */
void* EffectHeap::tryAlloc(size_t size, s32 alignment) {
    if (getMaxAllocatableSize(alignment) < size) {
        return nullptr;
    }

    return sead::ExpHeap::tryAlloc(size, alignment);
}

}  // namespace al
