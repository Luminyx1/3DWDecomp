#pragma once

#include <common/aglGPUCommon.hpp>
#include <common/aglGPUMemAddr.h>
#include <container/seadPtrArray.h>
#include <nn/gfx/gfx_Types.h>
#include <prim/seadSafeString.h>

namespace nn::g3d {
class ModelObj;
class ShadingModelObj;
}  // namespace nn::g3d

namespace sead {
class Heap;
}

namespace al {

class GpuMemAllocator {
public:
    struct Block {
        agl::GPUMemAddrBase addr;
        agl::GPUMemAddrBase tmpAddr;
        sead::FixedSafeString<32> name = {""};
        s32 usedSize = 0;
        s32 memorySize = 0;
        s32 tmpMemorySize = 0;
    };

    GpuMemAllocator(s32 memoryPoolNum, s32 modelObjNum, s32 shadingModelObjNum);
    ~GpuMemAllocator();

    void createMemory(const char* pName, s32 size, sead::Heap* pHeap, s32 alignment,
                      agl::MemoryAttribute attribute);
    Block* findGpuMemInfo(const char* pName) const;
    void createMemoryWithTmp(const char* pName, s32 size, s32 tmpSize, sead::Heap* pHeap,
                             s32 alignment, agl::MemoryAttribute attribute);
    agl::GPUMemAddrBase allocMemory(const char* pName, s32 size, s32 alignment);
    agl::GPUMemAddrBase getTmpMemoryAddr(const char* pName) const;
    u32 getTmpMemorySize(const char* pName) const;

    nn::gfx::MemoryPool* allocMemoryPool();
    bool registerModelObj(nn::g3d::ModelObj* pModelObj);
    bool registerShadingModelObj(nn::g3d::ShadingModelObj* pShadingModelObj);

private:
    sead::PtrArray<Block> mBlocks;
    sead::PtrArray<nn::g3d::ModelObj> mModelObjs;
    sead::PtrArray<nn::g3d::ShadingModelObj> mShadingModelObjs;
    nn::gfx::MemoryPool* mMemoryPools;
    s32 mMemoryPoolNum;
    s32 mMemoryPoolUsedNum = 0;
};

static_assert(sizeof(GpuMemAllocator::Block) == 0x78);
static_assert(sizeof(GpuMemAllocator) == 0x40);

}  // namespace al
