#pragma once

#include <basis/seadTypes.h>
#include <new>
#include <container/seadBuffer.h>
#include <prim/seadSafeString.h>
#include <thread/seadAtomic.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "layer/aglRenderDL.h"

namespace sead {
class Heap;
}

namespace agl {
class DrawContext;
}

namespace agl::lyr {

class RenderDLBuffer {
    friend class Renderer;
    friend class RenderDisplay;

public:
    static constexpr s32 cCoreNum = 3;

    RenderDLBuffer();
    virtual ~RenderDLBuffer();

    void initialize(u64 bufferSize, u64 controlMemorySize, s32 renderDLNum, s32 multiBufferNum,
                    sead::Heap* pHeap, sead::Heap* pDebugHeap);
    void clear(bool swapBuffer);
    void invalidateCurrentBuffer() const;
    s32 begin(DrawContext* pDrawContext, const sead::SafeString& rName, s32 priority);
    RenderDL* end(DrawContext* pDrawContext, s32 index);

private:
    struct Address : public GPUMemAddr<u8> {
        ~Address() {}
        Address& operator=(GPUMemAddr<u8> other)
        {
            new (this) GPUMemAddr<u8>(other);
            return *this;
        }
    };

    struct CoreBuffer {
        sead::Buffer<Address> mAddress;
        u32 mUsedSize = 0;
        u32 mBufferSize = 0;
        u8* mControlMemory = nullptr;
        u32 mControlMemoryUsed = 0;
        u32 mControlMemorySize = 0;
    };
    static_assert(sizeof(CoreBuffer) == 0x28);

    GPUMemBlock<u32> mGPUBuffer;
    GPUMemBlock<u32> mGPUBufferSub;
    void* mControlMemory;
    sead::Atomic<s32> mRenderDLNum;
    CoreBuffer mCoreBuffer[cCoreNum];
    sead::Buffer<RenderDL> mRenderDL;
    u8 mMultiBufferNum;
    u8 mCurrentBuffer;
    u8 _112;
    u64 mMaxUsedSize;
    u64 mMaxControlMemoryUsed;
    u64 _128;
};
static_assert(sizeof(RenderDLBuffer) == 0x130);

}  // namespace agl::lyr
