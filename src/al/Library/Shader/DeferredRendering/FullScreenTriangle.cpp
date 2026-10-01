#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"

#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglDrawContext.h"
#include "common/aglVertexAttribute.h"
#include "common/aglVertexBuffer.h"
#include "detail/aglMemoryPoolHeap.h"

#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Memory/Util.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
f32* getVertexPtr(const agl::GPUMemBlockBase& rBlock) {
    return reinterpret_cast<f32*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}
}  // namespace

namespace al {
/**
 * Creates the vertex buffers of the screen covering triangles.
 */
FullScreenTriangle::FullScreenTriangle() {
    mVertexMemBlock.allocBuffer(9, getCurrentHeap(), 8, agl::MemoryAttribute::Default);
    agl::GPUMemAddr<f32> addr(mVertexMemBlock, 0);
    mVertexMemBlockReverse.allocBuffer(9, getCurrentHeap(), 8, agl::MemoryAttribute::Default);
    agl::GPUMemAddr<f32> addrReverse(mVertexMemBlockReverse, 0);

    getVertexPtr(mVertexMemBlock)[0] = -1.0f;
    getVertexPtr(mVertexMemBlock)[1] = -1.0f;
    getVertexPtr(mVertexMemBlock)[2] = 1.0f;
    getVertexPtr(mVertexMemBlock)[3] = 3.0f;
    getVertexPtr(mVertexMemBlock)[4] = -1.0f;
    getVertexPtr(mVertexMemBlock)[5] = 1.0f;
    getVertexPtr(mVertexMemBlock)[6] = -1.0f;
    getVertexPtr(mVertexMemBlock)[7] = 3.0f;
    getVertexPtr(mVertexMemBlock)[8] = 1.0f;

    getVertexPtr(mVertexMemBlockReverse)[0] = -1.0f;
    getVertexPtr(mVertexMemBlockReverse)[1] = -1.0f;
    getVertexPtr(mVertexMemBlockReverse)[2] = 0.0f;
    getVertexPtr(mVertexMemBlockReverse)[3] = 3.0f;
    getVertexPtr(mVertexMemBlockReverse)[4] = -1.0f;
    getVertexPtr(mVertexMemBlockReverse)[5] = 0.0f;
    getVertexPtr(mVertexMemBlockReverse)[6] = -1.0f;
    getVertexPtr(mVertexMemBlockReverse)[7] = 3.0f;
    getVertexPtr(mVertexMemBlockReverse)[8] = 0.0f;

    mVertexBuffer = new agl::VertexBuffer();
    mVertexBuffer->setUpBuffer(agl::ConstGPUMemVoidAddr(mVertexMemBlock, 0), 0xc,
                               mVertexMemBlock.getSize());
    mVertexBuffer->setUpStream(0, agl::VertexStreamFormat(0x22), 0, false);
    mVertexAttribute = new agl::VertexAttribute();
    mVertexAttribute->create(1, nullptr);
    mVertexAttribute->setVertexStream(0, mVertexBuffer, 0);
    mVertexAttribute->setUp();

    mVertexBufferReverse = new agl::VertexBuffer();
    mVertexBufferReverse->setUpBuffer(agl::ConstGPUMemVoidAddr(mVertexMemBlockReverse, 0), 0xc,
                                      mVertexMemBlockReverse.getSize());
    mVertexBufferReverse->setUpStream(0, agl::VertexStreamFormat(0x22), 0, false);
    mVertexAttributeReverse = new agl::VertexAttribute();
    mVertexAttributeReverse->create(1, nullptr);
    mVertexAttributeReverse->setVertexStream(0, mVertexBufferReverse, 0);
    mVertexAttributeReverse->setUp();
}

/**
 * Destroys the vertex buffers.
 */
FullScreenTriangle::~FullScreenTriangle() {
    if (mVertexAttribute != nullptr) {
        delete mVertexAttribute;
        mVertexAttribute = nullptr;
    }

    if (mVertexBuffer != nullptr) {
        delete mVertexBuffer;
        mVertexBuffer = nullptr;
    }

    if (mVertexAttributeReverse != nullptr) {
        delete mVertexAttributeReverse;
        mVertexAttributeReverse = nullptr;
    }

    if (mVertexBufferReverse != nullptr) {
        delete mVertexBufferReverse;
        mVertexBufferReverse = nullptr;
    }

    mVertexMemBlock.free();
    mVertexMemBlockReverse.free();
}

/**
 * Draws a screen covering triangle at the far plane.
 * @param pDrawContext Draw context.
 */
void FullScreenTriangle::drawFar(agl::DrawContext* pDrawContext) const {
    if (isUsingReverseProjection()) {
        drawTriReverse(pDrawContext);
    } else {
        drawTri(pDrawContext);
    }
}

/**
 * Draws a screen covering triangle with reversed depth.
 * @param pDrawContext Draw context.
 */
void FullScreenTriangle::drawTriReverse(agl::DrawContext* pDrawContext) const {
    mVertexAttributeReverse->activate(pDrawContext);
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, 0, 9);
}

/**
 * Draws a screen covering triangle.
 * @param pDrawContext Draw context.
 */
void FullScreenTriangle::drawTri(agl::DrawContext* pDrawContext) const {
    mVertexAttribute->activate(pDrawContext);
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, 0, 9);
}

/**
 * Draws a screen covering triangle at the near plane.
 * @param pDrawContext Draw context.
 */
void FullScreenTriangle::drawNear(agl::DrawContext* pDrawContext) const {
    if (isUsingReverseProjection()) {
        drawTri(pDrawContext);
    } else {
        drawTriReverse(pDrawContext);
    }
}
}  // namespace al

namespace ShaderSearchImpl {
/**
 * Compares two strings.
 * @param pA First string.
 * @param pB Second string.
 * @return Whether the strings are equal.
 */
bool isEqualStr(const char* pA, const char* pB) {
    return al::isEqualString(pA, pB);
}
}  // namespace ShaderSearchImpl
