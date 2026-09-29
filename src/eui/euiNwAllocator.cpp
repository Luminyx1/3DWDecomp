#include <eui/euiNwAllocator.h>

#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>

namespace nn { namespace ui2d {
void Initialize(void* (*pAllocate)(size_t, size_t, void*),
                void (*pDeallocate)(void*, void*), void* pUserData);
} }

namespace eui {

/**
 * @brief Installs UI allocation callbacks using the specified heap.
 * @param[in] pHeap Heap used by the UI allocation callbacks.
 */
void NwAllocator::initialize(sead::Heap* pHeap) {
    nn::ui2d::Initialize(ui2dAllocateFunction, ui2dDeallocateFunction, pHeap);
}

/**
 * @brief Allocates UI memory from the heap supplied as callback data.
 * @param[in] size Number of bytes to allocate.
 * @param[in] alignment Alignment requested by the UI allocator.
 * @param[in] pUserData Heap pointer supplied when the callbacks were installed.
 */
void* NwAllocator::ui2dAllocateFunction(size_t size, size_t alignment, void* pUserData) {
    return static_cast<sead::Heap*>(pUserData)->alloc(size, static_cast<s32>(alignment));
}

/**
 * @brief Frees UI memory through the heap supplied as callback data.
 * @param[in] pMemory Memory block to release.
 * @param[in] pUserData Heap pointer supplied when the callbacks were installed.
 */
void NwAllocator::ui2dDeallocateFunction(void* pMemory, void* pUserData) {
    static_cast<sead::Heap*>(pUserData)->free(pMemory);
}

/** @brief Clears the UI allocation callbacks and their heap context. */
void NwAllocator::finalize() {
    nn::ui2d::Initialize(nullptr, nullptr, nullptr);
}

/**
 * @brief Frees UI memory if a containing sead heap can be found.
 * @param[in] pMemory Memory block to release.
 * @param[in] pUnused Unused callback context; the owning heap is found from the memory block.
 */
void NwAllocator::ui2dDeallocateFunctionWithFindContainHeap(void* pMemory, void* pUnused) {
    auto* pHeap = sead::HeapMgr::instance()->findContainHeap(pMemory);
    if (pHeap) {
        pHeap->free(pMemory);
    }
}

}  // namespace eui
