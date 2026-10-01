/**
 * @file Heap.h
 * @brief VFX heap implementation.
 */

#pragma once

#include <nn/types.h>

namespace nn {
namespace vfx {
class Heap {
public:
    virtual ~Heap() {}
    virtual void* Alloc(size_t size, size_t alignment) = 0;
    virtual void Free(void* ptr) = 0;
};
}  // namespace vfx
}  // namespace nn
