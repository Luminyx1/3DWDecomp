#pragma once

#include "common/aglGPUMemBlock.h"

namespace agl {
class DrawContext;
class VertexAttribute;
class VertexBuffer;
}  // namespace agl

namespace al {
class FullScreenTriangle {
public:
    FullScreenTriangle();
    ~FullScreenTriangle();

    void drawFar(agl::DrawContext* pDrawContext) const;
    void drawTriReverse(agl::DrawContext* pDrawContext) const;
    void drawTri(agl::DrawContext* pDrawContext) const;
    void drawNear(agl::DrawContext* pDrawContext) const;

private:
    agl::VertexAttribute* mVertexAttribute = nullptr;
    agl::VertexBuffer* mVertexBuffer = nullptr;
    agl::GPUMemBlock<f32> mVertexMemBlock;
    agl::VertexAttribute* mVertexAttributeReverse = nullptr;
    agl::VertexBuffer* mVertexBufferReverse = nullptr;
    agl::GPUMemBlock<f32> mVertexMemBlockReverse;
};

static_assert(sizeof(FullScreenTriangle) == 0x90);
}  // namespace al
