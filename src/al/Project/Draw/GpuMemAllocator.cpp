#include "Project/Draw/GpuMemAllocator.hpp"

#include <driver/aglGraphicsDriverMgr.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/gfx/gfx_MemoryPool.h>

#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

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

}  // namespace al
