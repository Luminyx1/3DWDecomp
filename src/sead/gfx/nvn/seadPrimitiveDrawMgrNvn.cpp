#include "gfx/nvn/seadPrimitiveDrawMgrNvn.h"

#include "filedevice/seadFileDeviceMgr.h"
#include "gfx/nin/seadGraphicsNvn.h"
#include "gfx/nvn/seadTextureNvn.h"
#include "gfx/seadDrawContext.h"
#include "gfx/seadPrimitiveRendererUtil.h"
#include "math/seadMatrixCalcCommon.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveDrawMgrNvn)

/**
 * Constructs the NVN primitive draw manager.
 */
PrimitiveDrawMgrNvn::PrimitiveDrawMgrNvn() = default;

/**
 * Destroys the NVN primitive draw manager.
 */
PrimitiveDrawMgrNvn::~PrimitiveDrawMgrNvn() = default;

/**
 * Creates the shader program, vertex states and all primitive vertex/index buffers.
 * @param pHeap heap to allocate from
 * @param pBinary shader binary
 * @param binarySize size of the shader binary
 */
void PrimitiveDrawMgrNvn::prepareFromBinaryImpl(Heap* pHeap, const void* pBinary, u32 binarySize)
{
    struct ShaderBinaryHeader
    {
        u32 vertexControlOffset;
        u32 fragmentControlOffset;
        u32 vertexDataOffset;
        u32 fragmentDataOffset;
    };

    NVNdevice* device = GraphicsNvn::instance()->getNvnDevice();
    nvnProgramInitialize(&mNvnProgram, device);

    size_t shaderSize;
    {
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                                       NVN_MEMORY_POOL_FLAGS_SHADER_CODE);
        shaderSize = size_t(binarySize) + 0xfff - (binarySize + 0xfff) % 0x1000;
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, const_cast<void*>(pBinary), shaderSize);
        nvnMemoryPoolInitialize(&mShaderMemoryPool, &poolBuilder);
    }

    NVNbufferBuilder bufferBuilder;
    nvnBufferBuilderSetDevice(&bufferBuilder, device);
    nvnBufferBuilderSetDefaults(&bufferBuilder);
    nvnBufferBuilderSetStorage(&bufferBuilder, &mShaderMemoryPool, 0, shaderSize);
    nvnBufferInitialize(&mShaderBuffer, &bufferBuilder);

    NVNbufferAddress address = nvnBufferGetAddress(&mShaderBuffer);
    const auto* header = static_cast<const ShaderBinaryHeader*>(pBinary);
    NVNshaderData shaderData[2];
    shaderData[0].data = address + header->vertexDataOffset;
    shaderData[0].control =
        reinterpret_cast<void*>(header->vertexControlOffset + uintptr_t(pBinary));
    shaderData[1].data = address + header->fragmentDataOffset;
    shaderData[1].control =
        reinterpret_cast<void*>(header->fragmentControlOffset + uintptr_t(pBinary));
    nvnProgramSetShaders(&mNvnProgram, 2, shaderData);

    nvnVertexAttribStateSetDefaults(&mVertexAttribStates[0]);
    nvnVertexAttribStateSetDefaults(&mVertexAttribStates[1]);
    nvnVertexAttribStateSetDefaults(&mVertexAttribStates[2]);
    nvnVertexAttribStateSetFormat(&mVertexAttribStates[0], NVN_FORMAT_RGB32F, 0);
    nvnVertexAttribStateSetFormat(&mVertexAttribStates[1], NVN_FORMAT_RG32F, 0xc);
    nvnVertexAttribStateSetFormat(&mVertexAttribStates[2], NVN_FORMAT_RGBA32F, 0x14);
    nvnVertexStreamStateSetDefaults(&mVertexStreamState);
    nvnVertexStreamStateSetStride(&mVertexStreamState, sizeof(PrimitiveDrawUtil::Vertex));

    {
        u32 bufferSize = mUniformBufferSize + 0x4000;
        size_t poolSize = size_t(bufferSize) + 0xfff - (bufferSize + 0xfff) % 0x1000;
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, new (pHeap, 0x1000) u8[poolSize], poolSize);
        nvnMemoryPoolInitialize(&mMemoryPool, &poolBuilder);
    }

    using Vertex = PrimitiveDrawUtil::Vertex;
    size_t offset = 0;

    setupNVNBuffer_(&mQuadVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * 4);
    setupNVNBuffer_(&mQuadIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 6);
    {
        auto* vertex = static_cast<Vertex*>(nvnBufferMap(&mQuadVertexBuffer));
        PrimitiveDrawUtil::setQuadVertex(vertex, static_cast<u16*>(nvnBufferMap(&mQuadIndexBuffer)));
        vertex[0].uv.set(0.0f, 0.0f);
        vertex[1].uv.set(1.0f, 0.0f);
        vertex[2].uv.set(0.0f, 1.0f);
        vertex[3].uv.set(1.0f, 1.0f);
    }

    setupNVNBuffer_(&mBoxIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 4);
    {
        auto* index = static_cast<u16*>(nvnBufferMap(&mBoxIndexBuffer));
        index[0] = 0;
        index[1] = 1;
        index[2] = 3;
        index[3] = 2;
    }

    setupNVNBuffer_(&mLineVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * 2);
    setupNVNBuffer_(&mLineIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 2);
    PrimitiveDrawUtil::setLineVertex(static_cast<Vertex*>(nvnBufferMap(&mLineVertexBuffer)),
                                     static_cast<u16*>(nvnBufferMap(&mLineIndexBuffer)));

    setupNVNBuffer_(&mCubeVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * 8);
    setupNVNBuffer_(&mCubeIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 36);
    PrimitiveDrawUtil::setCubeVertex(static_cast<Vertex*>(nvnBufferMap(&mCubeVertexBuffer)),
                                     static_cast<u16*>(nvnBufferMap(&mCubeIndexBuffer)));

    setupNVNBuffer_(&mWireCubeVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * 8);
    setupNVNBuffer_(&mWireCubeIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 17);
    PrimitiveDrawUtil::setWireCubeVertex(
        static_cast<Vertex*>(nvnBufferMap(&mWireCubeVertexBuffer)),
        static_cast<u16*>(nvnBufferMap(&mWireCubeIndexBuffer)));

    setupNVNBuffer_(&mSphereSVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * (8 * 4 + 2));
    setupNVNBuffer_(&mSphereSIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 8 * 4 * 6);
    PrimitiveDrawUtil::setSphereVertex(static_cast<Vertex*>(nvnBufferMap(&mSphereSVertexBuffer)),
                                       static_cast<u16*>(nvnBufferMap(&mSphereSIndexBuffer)), 8,
                                       4);

    setupNVNBuffer_(&mSphereLVertexBuffer, &mMemoryPool, &offset,
                    sizeof(Vertex) * (16 * 8 + 2));
    setupNVNBuffer_(&mSphereLIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 16 * 8 * 6);
    PrimitiveDrawUtil::setSphereVertex(static_cast<Vertex*>(nvnBufferMap(&mSphereLVertexBuffer)),
                                       static_cast<u16*>(nvnBufferMap(&mSphereLIndexBuffer)), 16,
                                       8);

    setupNVNBuffer_(&mDiskSVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * (16 + 1));
    setupNVNBuffer_(&mDiskSIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 16 * 3);
    PrimitiveDrawUtil::setDiskVertex(static_cast<Vertex*>(nvnBufferMap(&mDiskSVertexBuffer)),
                                     static_cast<u16*>(nvnBufferMap(&mDiskSIndexBuffer)), 16);

    setupNVNBuffer_(&mDiskLVertexBuffer, &mMemoryPool, &offset, sizeof(Vertex) * (32 + 1));
    setupNVNBuffer_(&mDiskLIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 32 * 3);
    PrimitiveDrawUtil::setDiskVertex(static_cast<Vertex*>(nvnBufferMap(&mDiskLVertexBuffer)),
                                     static_cast<u16*>(nvnBufferMap(&mDiskLIndexBuffer)), 32);

    setupNVNBuffer_(&mCircleSIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 16);
    {
        auto* index = static_cast<u16*>(nvnBufferMap(&mCircleSIndexBuffer));

        for (s32 i = 0; i < 16; i++)
        {
            index[i] = i;
        }
    }

    setupNVNBuffer_(&mCircleLIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 32);
    {
        auto* index = static_cast<u16*>(nvnBufferMap(&mCircleLIndexBuffer));

        for (s32 i = 0; i < 32; i++)
        {
            index[i] = i;
        }
    }

    setupNVNBuffer_(&mCylinderSVertexBuffer, &mMemoryPool, &offset,
                    sizeof(Vertex) * (16 + 1) * 2);
    setupNVNBuffer_(&mCylinderSIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 16 * 12);
    PrimitiveDrawUtil::setCylinderVertex(
        static_cast<Vertex*>(nvnBufferMap(&mCylinderSVertexBuffer)),
        static_cast<u16*>(nvnBufferMap(&mCylinderSIndexBuffer)), 16);

    setupNVNBuffer_(&mCylinderLVertexBuffer, &mMemoryPool, &offset,
                    sizeof(Vertex) * (32 + 1) * 2);
    setupNVNBuffer_(&mCylinderLIndexBuffer, &mMemoryPool, &offset, sizeof(u16) * 32 * 12);
    PrimitiveDrawUtil::setCylinderVertex(
        static_cast<Vertex*>(nvnBufferMap(&mCylinderLVertexBuffer)),
        static_cast<u16*>(nvnBufferMap(&mCylinderLIndexBuffer)), 32);

    offset = (offset + 0xff) & ~size_t(0xff);
    setupNVNBuffer_(&mUniformBuffer, &mMemoryPool, &offset, mUniformBufferSize);
    mUniformBufferMap = nvnBufferMap(&mUniformBuffer);
}

/**
 * Initializes a buffer in a memory pool and advances the pool offset.
 * @param pBuffer buffer to initialize
 * @param pMemoryPool memory pool to use as storage
 * @param pOffset current offset in the memory pool, advanced past the buffer
 * @param size size of the buffer
 */
void PrimitiveDrawMgrNvn::setupNVNBuffer_(NVNbuffer* pBuffer, NVNmemoryPool* pMemoryPool,
                                          size_t* pOffset, size_t size)
{
    NVNbufferBuilder bufferBuilder;
    nvnBufferBuilderSetDevice(&bufferBuilder, GraphicsNvn::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&bufferBuilder);
    nvnBufferBuilderSetStorage(&bufferBuilder, pMemoryPool, *pOffset, size);
    nvnBufferInitialize(pBuffer, &bufferBuilder);
    *pOffset = (size + *pOffset + 3) & ~size_t(3);
}

/**
 * Loads the shader binary from a file and prepares the manager from it.
 * @param pHeap heap to load into
 * @param rPath path of the shader binary
 */
void PrimitiveDrawMgrNvn::prepareImpl(Heap* pHeap, const SafeString& rPath)
{
    FileDevice::LoadArg loadArg;
    loadArg.path = rPath;
    loadArg.heap = pHeap;
    loadArg.alignment = 0x1000;
    loadArg.buffer_size_alignment = 0x1000;
    u8* binary = FileDeviceMgr::instance()->tryLoad(loadArg);
    prepareFromBinaryImpl(pHeap, binary, loadArg.read_size);
}

/**
 * Starts a new frame of the uniform ring buffer.
 */
void PrimitiveDrawMgrNvn::swapUniformBlockBuffer()
{
    mUniformBlockBuffer.swap(mUniformBlockBuffer.get_0(), mUniformBufferSize);
    mIsUniformBufferFull = false;
}

/**
 * Binds the primitive shader and uploads the view projection matrix.
 * @param pDrawContext draw context
 * @param rViewMatrix camera view matrix
 * @param rProjectionMatrix projection matrix
 */
void PrimitiveDrawMgrNvn::beginImpl(DrawContext* pDrawContext, const Matrix34f& rViewMatrix,
                                    const Matrix44f& rProjectionMatrix)
{
    if (mIsUniformBufferFull)
    {
        return;
    }

    u32 start = mUniformBlockBuffer.fetchAdd_0(0x100);

    if (start - mUniformBlockBuffer.get_4() + 0x100 > mUniformBufferSize)
    {
        mIsUniformBufferFull = true;
        return;
    }

    u32 offset = start % mUniformBufferSize;
    NVNcommandBuffer* commandBuffer = pDrawContext->getNvnCommandBuffer();
    nvnCommandBufferBindProgram(commandBuffer, &mNvnProgram, NVN_SHADER_STAGE_ALL_GRAPHICS_BITS);
    nvnCommandBufferBindVertexAttribState(commandBuffer, 3, mVertexAttribStates);
    nvnCommandBufferBindVertexStreamState(commandBuffer, 1, &mVertexStreamState);
    Matrix44CalcCommon<f32>::multiply(
        *reinterpret_cast<Matrix44f*>(static_cast<u8*>(mUniformBufferMap) + offset),
        rProjectionMatrix, rViewMatrix);
    nvnCommandBufferBindUniformBuffer(commandBuffer, NVN_SHADER_STAGE_VERTEX, 0,
                                      nvnBufferGetAddress(&mUniformBuffer) + offset, 0x40);
}

/**
 * Finishes drawing; does nothing.
 * @param pDrawContext draw context
 */
void PrimitiveDrawMgrNvn::endImpl(DrawContext* pDrawContext) {}

/**
 * Draws a quad.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawMgrNvn::drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                       const Color4f& rColor0, const Color4f& rColor1)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rColor0, rColor1, &mQuadVertexBuffer, 4, &mQuadIndexBuffer, 6, nullptr, nullptr,
              nullptr);
}

/**
 * Uploads the per draw uniforms and issues an indexed draw.
 * @param pCommandBuffer command buffer
 * @param primitive primitive type
 * @param rModelMatrix model matrix
 * @param rColor0 first color
 * @param rColor1 second color
 * @param pVertexBuffer vertex buffer
 * @param vertexNum number of vertices
 * @param pIndexBuffer index buffer
 * @param indexNum number of indices
 * @param pTexture texture, or nullptr
 * @param pUVSrc texture coordinate origin, or nullptr
 * @param pUVSize texture coordinate size, or nullptr
 */
void PrimitiveDrawMgrNvn::drawImpl_(NVNcommandBuffer* pCommandBuffer, NVNdrawPrimitive primitive,
                                    const Matrix34f& rModelMatrix, const Color4f& rColor0,
                                    const Color4f& rColor1, NVNbuffer* pVertexBuffer,
                                    u32 vertexNum, NVNbuffer* pIndexBuffer, u32 indexNum,
                                    const Texture* pTexture, const Vector2f* pUVSrc,
                                    const Vector2f* pUVSize)
{
    struct VertexUniform
    {
        Matrix44f modelMatrix;
        Color4f color0;
        Color4f color1;
        Vector4f uv;
    };

    if (mIsUniformBufferFull)
    {
        return;
    }

    nvnCommandBufferBindVertexBuffer(pCommandBuffer, 0, nvnBufferGetAddress(pVertexBuffer),
                                     sizeof(PrimitiveDrawUtil::Vertex) * vertexNum);

    if (mIsTextureEnable)
    {
        u32 start = mUniformBlockBuffer.fetchAdd_0(0x200);

        if (start - mUniformBlockBuffer.get_4() + 0x200 > mUniformBufferSize)
        {
            mIsUniformBufferFull = true;
            return;
        }

        u32 offset = start % mUniformBufferSize;
        auto* uniform =
            reinterpret_cast<VertexUniform*>(static_cast<u8*>(mUniformBufferMap) + offset);
        Matrix44CalcCommon<f32>::copy(uniform->modelMatrix, rModelMatrix, Vector4f::ew);
        uniform->color0 = rColor0;
        uniform->color1 = rColor1;

        if (pUVSrc && pUVSize)
        {
            uniform->uv.set(pUVSize->x, pUVSize->y, pUVSrc->x, pUVSrc->y);
        }
        else
        {
            uniform->uv.set(0.0f, 0.0f, 0.0f, 0.0f);
        }

        nvnCommandBufferBindUniformBuffer(pCommandBuffer, NVN_SHADER_STAGE_VERTEX, 1,
                                          nvnBufferGetAddress(&mUniformBuffer) + offset, 0x100);

        s32 textureOffset = offset + 0x100;
        auto* textureHandle = reinterpret_cast<NVNtextureHandle*>(
            textureOffset + uintptr_t(mUniformBufferMap));
        *textureHandle =
            (pTexture != nullptr) ? DynamicCast<const TextureNvn>(pTexture)->getTextureHandle() : 0;
        nvnCommandBufferBindUniformBuffer(pCommandBuffer, NVN_SHADER_STAGE_FRAGMENT, 0,
                                          nvnBufferGetAddress(&mUniformBuffer) + textureOffset, 8);
    }
    else
    {
        u32 start = mUniformBlockBuffer.fetchAdd_0(0x100);

        if (start - mUniformBlockBuffer.get_4() + 0x100 > mUniformBufferSize)
        {
            mIsUniformBufferFull = true;
            return;
        }

        u32 offset = start % mUniformBufferSize;
        auto* uniform =
            reinterpret_cast<VertexUniform*>(static_cast<u8*>(mUniformBufferMap) + offset);
        Matrix44CalcCommon<f32>::copy(uniform->modelMatrix, rModelMatrix, Vector4f::ew);
        uniform->color0 = rColor0;
        uniform->color1 = rColor1;
        nvnCommandBufferBindUniformBuffer(pCommandBuffer, NVN_SHADER_STAGE_VERTEX, 1,
                                          nvnBufferGetAddress(&mUniformBuffer) + offset, 0x60);
    }

    nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVN_INDEX_TYPE_UNSIGNED_SHORT,
                                 indexNum, nvnBufferGetAddress(pIndexBuffer));
}

/**
 * Draws a textured quad.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rTexture texture
 * @param rColor0 first color
 * @param rColor1 second color
 * @param rUVSrc texture coordinate origin
 * @param rUVSize texture coordinate size
 */
void PrimitiveDrawMgrNvn::drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                       const Texture& rTexture, const Color4f& rColor0,
                                       const Color4f& rColor1, const Vector2f& rUVSrc,
                                       const Vector2f& rUVSize)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rColor0, rColor1, &mQuadVertexBuffer, 4, &mQuadIndexBuffer, 6, &rTexture, &rUVSrc,
              &rUVSize);
}

/**
 * Draws a quad outline.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawMgrNvn::drawBoxImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                      const Color4f& rColor0, const Color4f& rColor1)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_LINE_LOOP, rModelMatrix,
              rColor0, rColor1, &mQuadVertexBuffer, 4, &mBoxIndexBuffer, 4, nullptr, nullptr,
              nullptr);
}

/**
 * Draws a cube.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawMgrNvn::drawCubeImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                       const Color4f& rColor0, const Color4f& rColor1)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rColor0, rColor1, &mCubeVertexBuffer, 8, &mCubeIndexBuffer, 36, nullptr, nullptr,
              nullptr);
}

/**
 * Draws a wireframe cube.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawMgrNvn::drawWireCubeImpl(DrawContext* pDrawContext,
                                           const Matrix34f& rModelMatrix, const Color4f& rColor0,
                                           const Color4f& rColor1)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_LINE_LOOP, rModelMatrix,
              rColor0, rColor1, &mWireCubeVertexBuffer, 8, &mWireCubeIndexBuffer, 17, nullptr,
              nullptr, nullptr);
}

/**
 * Draws a line.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rColor0 start color
 * @param rColor1 end color
 */
void PrimitiveDrawMgrNvn::drawLineImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                       const Color4f& rColor0, const Color4f& rColor1)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_LINE_LOOP, rModelMatrix,
              rColor0, rColor1, &mLineVertexBuffer, 2, &mLineIndexBuffer, 2, nullptr, nullptr,
              nullptr);
}

/**
 * Draws a sphere with 4x8 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawMgrNvn::drawSphere4x8Impl(DrawContext* pDrawContext,
                                            const Matrix34f& rModelMatrix, const Color4f& rNorth,
                                            const Color4f& rSouth)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rNorth, rSouth, &mSphereSVertexBuffer, 8 * 4 + 2, &mSphereSIndexBuffer, 8 * 4 * 6,
              nullptr, nullptr, nullptr);
}

/**
 * Draws a sphere with 8x16 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawMgrNvn::drawSphere8x16Impl(DrawContext* pDrawContext,
                                             const Matrix34f& rModelMatrix, const Color4f& rNorth,
                                             const Color4f& rSouth)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rNorth, rSouth, &mSphereLVertexBuffer, 16 * 8 + 2, &mSphereLIndexBuffer, 16 * 8 * 6,
              nullptr, nullptr, nullptr);
}

/**
 * Draws a disk with 16 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawMgrNvn::drawDisk16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                         const Color4f& rCenter, const Color4f& rEdge)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rCenter, rEdge, &mDiskSVertexBuffer, 16 + 1, &mDiskSIndexBuffer, 16 * 3, nullptr,
              nullptr, nullptr);
}

/**
 * Draws a disk with 32 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawMgrNvn::drawDisk32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                         const Color4f& rCenter, const Color4f& rEdge)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rCenter, rEdge, &mDiskLVertexBuffer, 32 + 1, &mDiskLIndexBuffer, 32 * 3, nullptr,
              nullptr, nullptr);
}

/**
 * Draws a circle with 16 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rEdge edge color
 */
void PrimitiveDrawMgrNvn::drawCircle16Impl(DrawContext* pDrawContext,
                                           const Matrix34f& rModelMatrix, const Color4f& rEdge)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_LINE_LOOP, rModelMatrix,
              rEdge, rEdge, &mDiskSVertexBuffer, 16 + 1, &mCircleSIndexBuffer, 16, nullptr,
              nullptr, nullptr);
}

/**
 * Draws a circle with 32 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rEdge edge color
 */
void PrimitiveDrawMgrNvn::drawCircle32Impl(DrawContext* pDrawContext,
                                           const Matrix34f& rModelMatrix, const Color4f& rEdge)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_LINE_LOOP, rModelMatrix,
              rEdge, rEdge, &mDiskLVertexBuffer, 32 + 1, &mCircleLIndexBuffer, 32, nullptr,
              nullptr, nullptr);
}

/**
 * Draws a cylinder with 16 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawMgrNvn::drawCylinder16Impl(DrawContext* pDrawContext,
                                             const Matrix34f& rModelMatrix, const Color4f& rTop,
                                             const Color4f& rBottom)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rTop, rBottom, &mCylinderSVertexBuffer, (16 + 1) * 2, &mCylinderSIndexBuffer,
              16 * 12, nullptr, nullptr, nullptr);
}

/**
 * Draws a cylinder with 32 divisions.
 * @param pDrawContext draw context
 * @param rModelMatrix model matrix
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawMgrNvn::drawCylinder32Impl(DrawContext* pDrawContext,
                                             const Matrix34f& rModelMatrix, const Color4f& rTop,
                                             const Color4f& rBottom)
{
    drawImpl_(pDrawContext->getNvnCommandBuffer(), NVN_DRAW_PRIMITIVE_TRIANGLES, rModelMatrix,
              rTop, rBottom, &mCylinderLVertexBuffer, (32 + 1) * 2, &mCylinderLIndexBuffer,
              32 * 12, nullptr, nullptr, nullptr);
}

}  // namespace sead
