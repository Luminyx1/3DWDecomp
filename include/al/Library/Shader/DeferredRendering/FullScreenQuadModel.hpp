#pragma once

#include "common/aglGPUMemBlock.h"
#include "common/aglVertexAttribute.h"

namespace agl {
class VertexBuffer;
}

namespace al {
class FullScreenQuadModel {
public:
    FullScreenQuadModel();
    ~FullScreenQuadModel();

    void drawQuad() const;

private:
    agl::VertexAttribute mVertexAttribute;
    agl::VertexBuffer* mVertexBuffer = nullptr;
    agl::GPUMemBlock<f32> mVertexMemBlock;
};

static_assert(sizeof(FullScreenQuadModel) == 0x220);
}  // namespace al
