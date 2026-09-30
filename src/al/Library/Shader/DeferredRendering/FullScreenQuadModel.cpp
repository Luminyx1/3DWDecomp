#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"

#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglDrawContext.h"
#include "common/aglVertexBuffer.h"
#include "detail/aglMemoryPoolHeap.h"

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"

namespace {
f32* getVertexPtr(const agl::GPUMemBlockBase& rBlock) {
    return reinterpret_cast<f32*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}
}  // namespace

namespace al {
/**
 * Creates the vertex buffer of two triangles covering the screen.
 */
FullScreenQuadModel::FullScreenQuadModel() {
    mVertexMemBlock.allocBuffer(18, getCurrentHeap(), 8, agl::MemoryAttribute::Default);
    agl::GPUMemAddr<f32> addr(mVertexMemBlock, 0);
    getVertexPtr(mVertexMemBlock)[0] = -1.0f;
    getVertexPtr(mVertexMemBlock)[1] = 1.0f;
    getVertexPtr(mVertexMemBlock)[2] = 1.0f;
    getVertexPtr(mVertexMemBlock)[3] = -1.0f;
    getVertexPtr(mVertexMemBlock)[4] = -1.0f;
    getVertexPtr(mVertexMemBlock)[5] = 1.0f;
    getVertexPtr(mVertexMemBlock)[6] = 1.0f;
    getVertexPtr(mVertexMemBlock)[7] = 1.0f;
    getVertexPtr(mVertexMemBlock)[8] = 1.0f;
    getVertexPtr(mVertexMemBlock)[9] = 1.0f;
    getVertexPtr(mVertexMemBlock)[10] = 1.0f;
    getVertexPtr(mVertexMemBlock)[11] = 1.0f;
    getVertexPtr(mVertexMemBlock)[12] = -1.0f;
    getVertexPtr(mVertexMemBlock)[13] = -1.0f;
    getVertexPtr(mVertexMemBlock)[14] = 1.0f;
    getVertexPtr(mVertexMemBlock)[15] = 1.0f;
    getVertexPtr(mVertexMemBlock)[16] = -1.0f;
    getVertexPtr(mVertexMemBlock)[17] = 1.0f;

    mVertexBuffer = new agl::VertexBuffer();
    mVertexBuffer->setUpBuffer(agl::ConstGPUMemVoidAddr(mVertexMemBlock, 0), 0xc,
                               mVertexMemBlock.getSize());
    mVertexBuffer->setUpStream(0, agl::VertexStreamFormat(0x22), 0, false);
    mVertexAttribute.create(1, nullptr);
    mVertexAttribute.setVertexStream(0, mVertexBuffer, 0);
    mVertexAttribute.setUp();
}

/**
 * Destroys the vertex buffer.
 */
FullScreenQuadModel::~FullScreenQuadModel() {
    if (mVertexBuffer) {
        delete mVertexBuffer;
        mVertexBuffer = nullptr;
    }
}

/**
 * Draws the two screen covering triangles.
 */
void FullScreenQuadModel::drawQuad() const {
    mVertexAttribute.activate(reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext));
    nvnCommandBufferDrawArrays(GameFrameworkNx::sInstance->mDrawContext->getNvnCommandBuffer(),
                               NVN_DRAW_PRIMITIVE_TRIANGLES, 0, 18);
}
}  // namespace al
