#include <nn/ui2d/ui2d_Layout.h>

namespace nn::ui2d {
// allocate and free implement layout memory management; argument is their context.
void Layout::SetAllocator(void* (*allocate)(size_t, size_t, void*), void (*free)(void*, void*), void* argument) {
    g_pAllocateFunction = reinterpret_cast<void*>(allocate);
    g_pFreeFunction = reinterpret_cast<void*>(free);
    g_pUserDataForAllocator = argument;
}

// size is the allocation size in bytes; alignment is the required byte alignment.
void* Layout::AllocateMemory(size_t size, size_t alignment) {
    return reinterpret_cast<void* (*)(size_t, size_t, void*)>(g_pAllocateFunction)(size, alignment, g_pUserDataForAllocator);
}

// size is the allocation size in bytes; ordinary layout allocations use alignment 4.
void* Layout::AllocateMemory(size_t size) { return AllocateMemory(size, 4); }
// memory is the allocation returned by the configured layout allocator.
void Layout::FreeMemory(void* memory) {
    reinterpret_cast<void (*)(void*, void*)>(g_pFreeFunction)(memory, g_pUserDataForAllocator);
}

// captureCount, vectorCount and dynamicCount bound shared texture records;
// stackCount bounds the nested parts-layout stack used while building them.
void Layout::SetDynamicTextureInitializationMemoryInfo(int captureCount, int vectorCount, int dynamicCount, int stackCount) {
    g_CaptureTextureShareInfoCountMax = captureCount;
    g_VectorGraphicsTextureShareInfoCountMax = vectorCount;
    g_DynamicTextureShareInfoCountMax = dynamicCount;
    g_DynamicTextureShareInfoPartsStackMax = stackCount;
}

Layout::Layout() : mRootPane(nullptr), _20(nullptr), _28(nullptr), _30(nullptr), _38(nullptr), mResourceAccessor(nullptr), _58(nullptr) {}
Layout::~Layout() = default;
}
