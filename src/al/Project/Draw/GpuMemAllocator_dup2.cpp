#include "Project/Draw/GpuMemAllocator.hpp"

#include <common/aglGPUMemBlock.h>

#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Allocates a named GPU memory block unless one with that name already exists.
 * @param pName Block name.
 * @param size Block size in bytes.
 * @param pHeap Heap to allocate from.
 * @param alignment Alignment of the block.
 * @param attribute Memory attribute of the block.
 */
void GpuMemAllocator::createMemory(const char* pName, s32 size, sead::Heap* pHeap, s32 alignment,
                                   agl::MemoryAttribute attribute) {
    if (findGpuMemInfo(pName)) {
        return;
    }

    Block* block = new Block;
    s64 longSize = size;
    auto* memBlock = new (pHeap) agl::GPUMemBlock<u8>;
    memBlock->allocBuffer_(longSize, pHeap, alignment, attribute);
    block->addr = agl::GPUMemAddrBase(*memBlock, 0);
    block->memorySize = size;
    block->name.format("%s", pName);

    mBlocks.pushBack(block);
}

/**
 * Finds a GPU memory block by name.
 * @param pName Block name.
 * @return The block, or nullptr if not found.
 */
GpuMemAllocator::Block* GpuMemAllocator::findGpuMemInfo(const char* pName) const {
    for (s32 i = 0; i < mBlocks.size(); i++) {
        Block* block = mBlocks[i];
        if (isEqualString(pName, block->name.cstr())) {
            return block;
        }
    }

    return nullptr;
}

/**
 * Allocates a named GPU memory block together with a temporary block.
 * @param pName Block name.
 * @param size Block size in bytes.
 * @param tmpSize Temporary block size in bytes.
 * @param pHeap Heap to allocate from.
 * @param alignment Alignment of the blocks.
 * @param attribute Memory attribute of the blocks.
 */
void GpuMemAllocator::createMemoryWithTmp(const char* pName, s32 size, s32 tmpSize,
                                          sead::Heap* pHeap, s32 alignment,
                                          agl::MemoryAttribute attribute) {
    createMemory(pName, size, pHeap, alignment, attribute);
    Block* block = findGpuMemInfo(pName);
    s64 longSize = tmpSize;
    auto* memBlock = new (pHeap) agl::GPUMemBlock<u8>;
    memBlock->allocBuffer_(longSize, pHeap, alignment, attribute);
    block->tmpAddr = agl::GPUMemAddrBase(*memBlock, 0);
    block->tmpMemorySize = tmpSize;
}

/**
 * Sub-allocates memory from a named GPU memory block.
 * @param pName Block name.
 * @param size Size in bytes.
 * @param alignment Alignment of the allocation.
 * @return The allocated address, or an invalid address on failure.
 */
agl::GPUMemAddrBase GpuMemAllocator::allocMemory(const char* pName, s32 size, s32 alignment) {
    Block* block = findGpuMemInfo(pName);
    if (!block) {
        return {};
    }

    if (block->usedSize + size > block->memorySize) {
        return {};
    }

    s32 alignedSize;
    if (block->usedSize < 0) {
        alignedSize = (block->usedSize - alignment + 1) / alignment * alignment;
    } else {
        alignedSize = (block->usedSize + alignment - 1) / alignment * alignment;
    }

    block->usedSize = alignedSize + size;
    return {block->addr, alignedSize};
}

/**
 * Gets the temporary memory of a named GPU memory block after invalidating its CPU cache.
 * @param pName Block name.
 * @return The temporary memory address, or an invalid address if not found.
 */
agl::GPUMemAddrBase GpuMemAllocator::getTmpMemoryAddr(const char* pName) const {
    Block* block = findGpuMemInfo(pName);
    if (!block) {
        return {};
    }

    block->tmpAddr.invalidateCPUCache(block->tmpMemorySize);
    return {block->tmpAddr, 0};
}

/**
 * Gets the temporary memory size of a named GPU memory block.
 * @param pName Block name.
 * @return The temporary memory size, or 0 if not found.
 */
u32 GpuMemAllocator::getTmpMemorySize(const char* pName) const {
    Block* block = findGpuMemInfo(pName);
    if (!block) {
        return 0;
    }

    return block->tmpMemorySize;
}

}  // namespace al
