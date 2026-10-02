#include "Project/Draw/GpuMemAllocator.hpp"

#include <common/aglGPUMemBlock.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/gfx/gfx_MemoryPool.h>

#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Constructs the allocator and its memory pool, model object and shading model object tables.
 * @param memoryPoolNum Number of memory pools.
 * @param modelObjNum Maximum number of registered model objects.
 * @param shadingModelObjNum Maximum number of registered shading model objects.
 */
GpuMemAllocator::GpuMemAllocator(s32 memoryPoolNum, s32 modelObjNum, s32 shadingModelObjNum)
    : mMemoryPoolNum(memoryPoolNum) {
    ShaderHolder::sInstance->setupShaderArchives();
    mBlocks.allocBuffer(16, nullptr);
    mModelObjs.allocBuffer(modelObjNum, nullptr);
    mShadingModelObjs.allocBuffer(shadingModelObjNum, nullptr);
    mMemoryPools = new nn::gfx::MemoryPool[mMemoryPoolNum];
}

/**
 * Frees all GPU memory blocks, block buffers and memory pools.
 */
GpuMemAllocator::~GpuMemAllocator() {
    for (s32 i = 0; i < mBlocks.size(); i++) {
        Block* block = mBlocks[i];

        if (block->addr.isValid()) {
            block->addr.deleteGPUMemBlock();
            block->addr.invalidate();
        }

        if (block->tmpAddr.isValid()) {
            block->tmpAddr.deleteGPUMemBlock();
            block->tmpAddr.invalidate();
        }
    }

    for (s32 i = 0; i < mModelObjs.size(); i++) {
        nn::g3d::ModelObj* modelObj = mModelObjs[i];

        if (modelObj->IsBlockBufferValid()) {
            modelObj->CleanupBlockBuffer(static_cast<nn::gfx::Device*>(
                agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()));
        }
    }

    for (s32 i = 0; i < mShadingModelObjs.size(); i++) {
        nn::g3d::ShadingModelObj* shadingModelObj = mShadingModelObjs[i];

        if (shadingModelObj->IsBlockBufferValid()) {
            shadingModelObj->CleanupBlockBuffer(static_cast<nn::gfx::Device*>(
                agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()));
        }
    }

    ShaderHolder::sInstance->cleanupShaderArchives();
    delete[] mMemoryPools;
}

/**
 * Takes the next unused memory pool.
 * @return The memory pool, or nullptr if all are in use.
 */
nn::gfx::MemoryPool* GpuMemAllocator::allocMemoryPool() {
    if (mMemoryPoolUsedNum >= mMemoryPoolNum) {
        return nullptr;
    }

    return &mMemoryPools[mMemoryPoolUsedNum++];
}

/**
 * Registers a model object whose block buffer is cleaned up on destruction.
 * @param pModelObj Model object.
 * @return Always true.
 */
bool GpuMemAllocator::registerModelObj(nn::g3d::ModelObj* pModelObj) {
    mModelObjs.pushBack(pModelObj);
    return true;
}

/**
 * Registers a shading model object whose block buffer is cleaned up on destruction.
 * @param pShadingModelObj Shading model object.
 * @return Always true.
 */
bool GpuMemAllocator::registerShadingModelObj(nn::g3d::ShadingModelObj* pShadingModelObj) {
    mShadingModelObjs.pushBack(pShadingModelObj);
    return true;
}

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
    if (findGpuMemInfo(pName) != nullptr) {
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

    if (block == nullptr) {
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

    if (block == nullptr) {
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

    if (block == nullptr) {
        return 0;
    }

    return block->tmpMemorySize;
}

}  // namespace al
