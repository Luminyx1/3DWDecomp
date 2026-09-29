#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglTextureData.h"
#include "common/aglTextureEnum.h"

namespace sead {
class Heap;
}  // namespace sead

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::utl {

class DynamicTextureCache {
public:
    enum AllocateType {
        cAllocateType_0 = 0,
        cAllocateType_1 = 1,
        cAllocateType_2 = 2,
        cAllocateType_3 = 3,
    };

    DynamicTextureCache();
    virtual ~DynamicTextureCache();

    void initialize(s32 num, sead::Heap* pHeap);
    bool begin();
    void end();

    TextureData* alloc(DrawContext* pDrawContext, const sead::SafeString& rName,
                       TextureFormat format, u32 width, u32 height, u32 mipLevelNum,
                       GPUMemVoidAddr* pAddr, AllocateType type, bool b);
    TextureData* allocArray(DrawContext* pDrawContext, const sead::SafeString& rName,
                            TextureFormat format, u32 width, u32 height, u32 arrayNum,
                            u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b);
    TextureData* alloc3D(DrawContext* pDrawContext, const sead::SafeString& rName,
                         TextureFormat format, u32 width, u32 height, u32 depth, u32 mipLevelNum,
                         GPUMemVoidAddr* pAddr, AllocateType type, bool b);
    TextureData* allocCube(DrawContext* pDrawContext, const sead::SafeString& rName,
                           TextureFormat format, u32 width, u32 mipLevelNum,
                           GPUMemVoidAddr* pAddr, AllocateType type, bool b);
    TextureData* allocCubeArray(DrawContext* pDrawContext, const sead::SafeString& rName,
                                TextureFormat format, u32 width, u32 arrayNum, u32 mipLevelNum,
                                GPUMemVoidAddr* pAddr, AllocateType type, bool b);
    TextureData* allocMultiSample(DrawContext* pDrawContext, const sead::SafeString& rName,
                                  TextureFormat format, u32 width, u32 height,
                                  MultiSampleType multiSample, GPUMemVoidAddr* pAddr,
                                  AllocateType type, bool b);
    void free(TextureData* pTexture);

private:
    void pushBack_(TextureData* pTexture);
    TextureData* popFront_(DrawContext* pDrawContext);

    sead::BitFlag8 mFlags;
    sead::PtrArray<TextureData> mCache;
    sead::Buffer<TextureData> mBuffer;
};
static_assert(sizeof(DynamicTextureCache) == 0x30);

}  // namespace agl::utl
