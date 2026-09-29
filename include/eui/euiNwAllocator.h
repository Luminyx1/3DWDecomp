#pragma once

#include <cstddef>

namespace sead {
class Heap;
}

namespace eui {

class NwAllocator {
public:
    static void initialize(sead::Heap* pHeap);
    static void* ui2dAllocateFunction(size_t size, size_t alignment, void* pUserData);
    static void ui2dDeallocateFunction(void* pMemory, void* pUserData);
    static void finalize();
    static void ui2dDeallocateFunctionWithFindContainHeap(void* pMemory, void* pUserData);
};

}  // namespace eui
