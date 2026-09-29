#pragma once

#include <mc/seadCoreInfo.h>
#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglDrawContext.h"
#include "common/aglIndexStream.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::pfx::detail {

inline void drawIndexStream(DrawContext* pDrawContext, const IndexStream& rStream)
{
    u32 count = rStream.getCount();
    if (count == 0)
    {
        return;
    }
    NVNdrawPrimitive primitive = rStream.getPrimitiveType();
    NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
    NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
    nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVNindexType(rStream.getFormat()),
                                 count, address);
}

inline void drawQuad(DrawContext* pDrawContext)
{
    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_Quad)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, utl::PrimitiveShape::instance()->getQuadIndexStream());
}

inline bool isDynamicTextureCaching()
{
    const auto* allocator = reinterpret_cast<const u8*>(utl::DynamicTextureAllocator::instance());
    s32 core = sead::CoreInfo::getCurrentCoreId();
    return *reinterpret_cast<const u32*>(allocator + 0x2ac + core * 0x288) & 1;
}

}  // namespace agl::pfx::detail

namespace agl::pfx::detail {

template <typename T>
inline T* getBufferPtr(const GPUMemBlockBase& rBlock)
{
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}

inline void drawQuadTriangle(DrawContext* pDrawContext)
{
    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

}  // namespace agl::pfx::detail

#include <arm_neon.h>
#include <math/seadMatrix.h>

namespace agl::pfx::detail {

inline void multiplyMtx44(sead::Matrix44f& rOut, const sead::Matrix44f& rA,
                          const sead::Matrix44f& rB)
{
    const float32x4_t a0 = vld1q_f32(rA.m[0]);
    const float32x4_t a1 = vld1q_f32(rA.m[1]);
    const float32x4_t a2 = vld1q_f32(rA.m[2]);
    const float32x4_t a3 = vld1q_f32(rA.m[3]);

    const float32x4_t b0 = vld1q_f32(rB.m[0]);
    const float32x4_t b1 = vld1q_f32(rB.m[1]);
    const float32x4_t b2 = vld1q_f32(rB.m[2]);
    const float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);

    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);

    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);

    float32x4_t c3 = vmulq_laneq_f32(b0, a3, 0);
    c3 = vfmaq_laneq_f32(c3, b1, a3, 1);
    c3 = vfmaq_laneq_f32(c3, b2, a3, 2);
    c3 = vfmaq_laneq_f32(c3, b3, a3, 3);

    vst1q_f32(rOut.m[0], c0);
    vst1q_f32(rOut.m[1], c1);
    vst1q_f32(rOut.m[2], c2);
    vst1q_f32(rOut.m[3], c3);
}

}  // namespace agl::pfx::detail
