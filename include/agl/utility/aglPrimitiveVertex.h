#pragma once

#include <heap/seadDisposer.h>
#include <math/seadVector.h>
#include "common/aglGPUMemBlock.h"
#include "common/aglVertexBuffer.h"

namespace agl::utl {

class PrimitiveVertex {
    SEAD_SINGLETON_DISPOSER(PrimitiveVertex)

    PrimitiveVertex();
    virtual ~PrimitiveVertex();

public:
    void initialize(sead::Heap* pHeap);

    enum Type
    {
        cType_White,
        cType_Black,
        cType_Zero,
        cType_Num
    };

    const VertexBuffer& getVertexBuffer(Type type) const { return mVertexBuffers[type]; }

private:
    VertexBuffer mVertexBuffers[cType_Num];
    GPUMemBlock<sead::Vector4f> mBlocks[cType_Num];
};
static_assert(sizeof(PrimitiveVertex) == 0x430);

}  // namespace agl::utl
