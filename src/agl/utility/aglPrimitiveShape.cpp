#include "utility/aglPrimitiveShape.h"
#include <cmath>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>

namespace agl::utl
{

namespace
{

template <typename T>
T* getBufferPtr_(const GPUMemBlock<T>& rBlock)
{
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}

u32 rad2idx_(f32 rad)
{
    return static_cast<u32>(
        static_cast<s64>(rad * (sead::Mathf::cHalfRoundIdx / sead::Mathf::pi())));
}

template <typename T>
void setupIdxStreamGrid_(IndexStream* pIndexStream, GPUMemAddr<T> addr, s32 num)
{
    T* indices = static_cast<T*>(addr.getPtr());

    for (s32 i = 0; i < num; i++)
    {
        indices[i] = i;
    }

    pIndexStream->setUpStream(addr, num);
    pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
}

template <typename T>
void checkAddr_(GPUMemAddr<T>)
{
}

template <typename T, s32 N>
void setVertices_(GPUMemBlock<T>& rBlock, T (&rVertices)[N])
{
    for (s32 i = 0; i < N; i++)
    {
        getBufferPtr_(rBlock)[i].mPos = rVertices[i].mPos;
        getBufferPtr_(rBlock)[i].mNormal = rVertices[i].mNormal;
        getBufferPtr_(rBlock)[i].mTexCoord = rVertices[i].mTexCoord;
    }
}

template <typename T, s32 N>
void setIndices_(GPUMemBlock<T>& rBlock, const T (&rIndices)[N])
{
    for (s32 i = 0; i < N; i++)
    {
        getBufferPtr_(rBlock)[i] = rIndices[i];
    }
}

}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveShape)

/**
 * Constructs the primitive shapes without any buffers.
 */
PrimitiveShape::PrimitiveShape() = default;

/**
 * Destroys the primitive shapes.
 */
PrimitiveShape::~PrimitiveShape() = default;

/**
 * Creates the vertex and index buffers of every primitive shape.
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::initialize(sead::Heap* pHeap)
{
    setUpStreamQuad_(pHeap);
    setUpStreamQuadTriangle_(pHeap);
    setUpStreamCube_(pHeap);
    setUpStreamPyramid_(pHeap);
    setUpStreamCircle_(32, pHeap);
    setUpStreamSphere_(32, 16, pHeap);
    setUpStreamHemisphere_(32, 16, pHeap);
    setUpStreamCylinder_(32, 16, pHeap);
    setUpStreamCapsule_(32, 8, 8, pHeap);
    setUpStreamCone_(32, 16, pHeap);
    setUpStreamTorus_(32, 32, pHeap, 1.0f / 3.0f, 1.0f / 6.0f, 0, 1);
}

/**
 * Creates the buffers of the quad.
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamQuad_(sead::Heap* pHeap)
{
    mQuadVertexBlock.allocBuffer(4, pHeap, 8, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mQuadVertexBlock, 0));
    {
        Vertex vertices[4] = {
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        };

        for (s32 i = 0; i < 4; i++)
        {
            getBufferPtr_(mQuadVertexBlock)[i].mPos = vertices[i].mPos;
            getBufferPtr_(mQuadVertexBlock)[i].mNormal = vertices[i].mNormal;
            getBufferPtr_(mQuadVertexBlock)[i].mTexCoord = vertices[i].mTexCoord;
        }
    }

    mQuadVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mQuadVertexBlock, 0), sizeof(Vertex),
                                  sizeof(Vertex) * 4);
    setUpStreams_(&mQuadVertexBuffer);

    mQuadTriangleIndexBlock.allocBuffer(6, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mQuadTriangleIndexBlock, 0));
    const u16 cTriangleIndices[6] = {0, 2, 1, 1, 2, 3};
    setIndices_(mQuadTriangleIndexBlock, cTriangleIndices);
    mQuadTriangleIndexStream.setUpStream(GPUMemAddr<u16>(mQuadTriangleIndexBlock, 0), 6);

    mQuadLineIndexBlock.allocBuffer(4, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mQuadLineIndexBlock, 0));
    const u16 cLineIndices[4] = {0, 1, 3, 2};
    setIndices_(mQuadLineIndexBlock, cLineIndices);
    mQuadLineIndexStream.setUpStream(GPUMemAddr<u16>(mQuadLineIndexBlock, 0), 4);
    mQuadLineIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINE_LOOP);
}

/**
 * Creates the buffers of the triangle covering a quad.
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamQuadTriangle_(sead::Heap* pHeap)
{
    mQuadTriangleVertexBlock.allocBuffer(3, pHeap, 8, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mQuadTriangleVertexBlock, 0));
    {
        Vertex vertices[3] = {
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {2.0f, 0.0f}},
            {{-0.5f, -1.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 2.0f}},
        };

        for (s32 i = 0; i < 3; i++)
        {
            getBufferPtr_(mQuadTriangleVertexBlock)[i].mPos = vertices[i].mPos;
            getBufferPtr_(mQuadTriangleVertexBlock)[i].mNormal = vertices[i].mNormal;
            getBufferPtr_(mQuadTriangleVertexBlock)[i].mTexCoord = vertices[i].mTexCoord;
        }
    }

    mQuadTriangleVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mQuadTriangleVertexBlock, 0),
                                            sizeof(Vertex), sizeof(Vertex) * 3);
    setUpStreams_(&mQuadTriangleVertexBuffer);

    mQuadTriangleTriangleIndexBlock.allocBuffer(3, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mQuadTriangleTriangleIndexBlock, 0));
    getBufferPtr_(mQuadTriangleTriangleIndexBlock)[0] = 0;
    getBufferPtr_(mQuadTriangleTriangleIndexBlock)[1] = 2;
    getBufferPtr_(mQuadTriangleTriangleIndexBlock)[2] = 1;
    mQuadTriangleTriangleIndexStream.setUpStream(GPUMemAddr<u16>(mQuadTriangleTriangleIndexBlock, 0),
                                               3);

    mQuadTriangleLineIndexBlock.allocBuffer(3, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mQuadTriangleLineIndexBlock, 0));
    getBufferPtr_(mQuadTriangleLineIndexBlock)[0] = 0;
    getBufferPtr_(mQuadTriangleLineIndexBlock)[1] = 2;
    getBufferPtr_(mQuadTriangleLineIndexBlock)[2] = 1;
    mQuadTriangleLineIndexStream.setUpStream(GPUMemAddr<u16>(mQuadTriangleLineIndexBlock, 0),
                                               3);
    mQuadTriangleLineIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINE_LOOP);
}

/**
 * Creates the buffers of the cube.
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamCube_(sead::Heap* pHeap)
{
    mCubeVertexBlock.allocBuffer(24, pHeap, 8, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mCubeVertexBlock, 0));
    {
        Vertex vertices[24] = {
            {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        };

        for (s32 i = 0; i < 24; i++)
        {
            getBufferPtr_(mCubeVertexBlock)[i].mPos = vertices[i].mPos;
            getBufferPtr_(mCubeVertexBlock)[i].mNormal = vertices[i].mNormal;
            getBufferPtr_(mCubeVertexBlock)[i].mTexCoord = vertices[i].mTexCoord;
        }
    }

    mCubeVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mCubeVertexBlock, 0), sizeof(Vertex),
                                  sizeof(Vertex) * 24);
    setUpStreams_(&mCubeVertexBuffer);

    mCubeTriangleIndexBlock.allocBuffer(36, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mCubeTriangleIndexBlock, 0));
    static const u16 cTriangleIndices[36] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23};
    setIndices_(mCubeTriangleIndexBlock, cTriangleIndices);
    mCubeTriangleIndexStream.setUpStream(GPUMemAddr<u16>(mCubeTriangleIndexBlock, 0), 36);

    mCubeLineIndexBlock.allocBuffer(48, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mCubeLineIndexBlock, 0));
    static const u16 cLineIndices[48] = {0, 1, 0, 2, 3, 1, 3, 2, 4, 5, 4, 6, 7, 5, 7, 6, 8, 9, 8, 10, 11, 9, 11, 10, 12, 13, 12, 14, 15, 13, 15, 14, 16, 17, 16, 18, 19, 17, 19, 18, 20, 21, 20, 22, 23, 21, 23, 22};
    setIndices_(mCubeLineIndexBlock, cLineIndices);
    mCubeLineIndexStream.setUpStream(GPUMemAddr<u16>(mCubeLineIndexBlock, 0), 48);
    mCubeLineIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
}

/**
 * Creates the buffers of the pyramid.
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamPyramid_(sead::Heap* pHeap)
{
    mPyramidVertexBlock.allocBuffer(16, pHeap, 8, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mPyramidVertexBlock, 0));
    {
        Vertex vertices[16] = {
            {{0.0f, -0.0f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.5f, 1.0f}},
            {{0.5f, -0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.5f}},
            {{-0.5f, -0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.5f}},
            {{0.0f, -0.0f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.0f}},
            {{0.0f, -0.0f, -0.5f}, {-0.5773503f, 0.5773503f, -0.5773503f}, {0.0f, 1.0f}},
            {{-0.5f, -0.0f, 0.0f}, {-0.5773503f, 0.5773503f, -0.5773503f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f, 0.0f}, {-0.5773503f, 0.5773503f, -0.5773503f}, {0.5f, 0.0f}},
            {{-0.5f, -0.0f, 0.0f}, {-0.5773503f, 0.5773503f, 0.5773503f}, {0.0f, 1.0f}},
            {{0.0f, -0.0f, 0.5f}, {-0.5773503f, 0.5773503f, 0.5773503f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f, 0.0f}, {-0.5773503f, 0.5773503f, 0.5773503f}, {0.5f, 0.0f}},
            {{0.0f, -0.0f, 0.5f}, {0.5773503f, 0.5773503f, 0.5773503f}, {0.0f, 1.0f}},
            {{0.5f, -0.0f, 0.0f}, {0.5773503f, 0.5773503f, 0.5773503f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f, 0.0f}, {0.5773503f, 0.5773503f, 0.5773503f}, {0.5f, 0.0f}},
            {{0.5f, -0.0f, 0.0f}, {0.5773503f, 0.5773503f, -0.5773503f}, {0.0f, 1.0f}},
            {{0.0f, -0.0f, -0.5f}, {0.5773503f, 0.5773503f, -0.5773503f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f, 0.0f}, {0.5773503f, 0.5773503f, -0.5773503f}, {0.5f, 0.0f}},
        };

        for (s32 i = 0; i < 16; i++)
        {
            getBufferPtr_(mPyramidVertexBlock)[i].mPos = vertices[i].mPos;
            getBufferPtr_(mPyramidVertexBlock)[i].mNormal = vertices[i].mNormal;
            getBufferPtr_(mPyramidVertexBlock)[i].mTexCoord = vertices[i].mTexCoord;
        }
    }

    mPyramidVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mPyramidVertexBlock, 0), sizeof(Vertex),
                                  sizeof(Vertex) * 16);
    setUpStreams_(&mPyramidVertexBuffer);

    mPyramidTriangleIndexBlock.allocBuffer(18, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mPyramidTriangleIndexBlock, 0));
    static const u16 cTriangleIndices[18] = {0, 1, 2, 1, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    setIndices_(mPyramidTriangleIndexBlock, cTriangleIndices);
    mPyramidTriangleIndexStream.setUpStream(GPUMemAddr<u16>(mPyramidTriangleIndexBlock, 0), 18);

    mPyramidLineIndexBlock.allocBuffer(32, pHeap, 4, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<u16>(mPyramidLineIndexBlock, 0));
    static const u16 cLineIndices[32] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 4, 7, 8, 8, 9, 9, 7, 10, 11, 11, 12, 12, 10, 13, 14, 14, 15, 15, 13};
    setIndices_(mPyramidLineIndexBlock, cLineIndices);
    mPyramidLineIndexStream.setUpStream(GPUMemAddr<u16>(mPyramidLineIndexBlock, 0), 32);
    mPyramidLineIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
}

/**
 * Creates the buffers of the circle.
 * @param divNum number of divisions of the circumference
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamCircle_(u32 divNum, sead::Heap* pHeap)
{
    mCircleVertexBlock.allocBuffer(calcVtxArrayNumCircle(divNum), pHeap, 8, MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mCircleVertexBlock, 0));
    setupVtxBufferCircle(&mCircleVertexBuffer, GPUMemAddr<Vertex>(mCircleVertexBlock, 0), divNum);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        for (s32 drawType = 0; drawType < cDrawType_Num; drawType++)
        {
            GPUMemBlock<u16>& rBlock = mCircleIndexBlocks[quality][drawType];
            rBlock.allocBuffer(
                calcIdxArrayNumCircle_(divNum, DrawType(drawType), Quality(quality)), pHeap, 4,
                MemoryAttribute(0));
            checkAddr_(GPUMemAddr<u16>(rBlock, 0));
            setupIdxStreamCircle_(&mCircleIndexStreams[quality][drawType],
                                  GPUMemAddr<u16>(rBlock, 0), divNum, DrawType(drawType),
                                  Quality(quality));
        }
    }
}

/**
 * Creates the buffers of the sphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamSphere_(u32 divNumU, u32 divNumV, sead::Heap* pHeap)
{
    mSphereVertexBlock.allocBuffer(calcVtxArrayNumSphere(divNumU, divNumV), pHeap, 8,
                                   MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mSphereVertexBlock, 0));
    setupVtxBufferSphere(&mSphereVertexBuffer, GPUMemAddr<Vertex>(mSphereVertexBlock, 0), divNumU,
                         divNumV);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        for (s32 drawType = 0; drawType < cDrawType_Num; drawType++)
        {
            GPUMemBlock<u16>& rBlock = mSphereIndexBlocks[quality][drawType];
            rBlock.allocBuffer(calcIdxArrayNumSphere_(divNumU, divNumV, DrawType(drawType),
                                                      Quality(quality)),
                               pHeap, 4, MemoryAttribute(0));
            checkAddr_(GPUMemAddr<u16>(rBlock, 0));
            setupIdxStreamSphere_(&mSphereIndexStreams[quality][drawType],
                                  GPUMemAddr<u16>(rBlock, 0), divNumU, divNumV, DrawType(drawType),
                                  Quality(quality));
        }
    }
}

/**
 * Creates the buffers of the hemisphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamHemisphere_(u32 divNumU, u32 divNumV, sead::Heap* pHeap)
{
    mHemisphereVertexBlock.allocBuffer(calcVtxArrayNumHemisphere(divNumU, divNumV), pHeap, 8,
                                       MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mHemisphereVertexBlock, 0));
    setupVtxBufferHemisphere(&mHemisphereVertexBuffer,
                             GPUMemAddr<Vertex>(mHemisphereVertexBlock, 0), divNumU, divNumV);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        for (s32 drawType = 0; drawType < cDrawType_Num; drawType++)
        {
            GPUMemBlock<u16>& rBlock = mHemisphereIndexBlocks[quality][drawType];
            rBlock.allocBuffer(calcIdxArrayNumHemisphere_(divNumU, divNumV, DrawType(drawType),
                                                          Quality(quality)),
                               pHeap, 4, MemoryAttribute(0));
            checkAddr_(GPUMemAddr<u16>(rBlock, 0));
            setupIdxStreamHemisphere_(&mHemisphereIndexStreams[quality][drawType],
                                      GPUMemAddr<u16>(rBlock, 0), divNumU, divNumV,
                                      DrawType(drawType), Quality(quality));
        }
    }
}

/**
 * Creates the buffers of the cylinder.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamCylinder_(u32 divNumU, u32 divNumV, sead::Heap* pHeap)
{
    mCylinderVertexBlock.allocBuffer(calcVtxArrayNumCylinder(divNumU, divNumV), pHeap, 8,
                                     MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mCylinderVertexBlock, 0));
    setupVtxBufferCylinder(&mCylinderVertexBuffer, GPUMemAddr<Vertex>(mCylinderVertexBlock, 0),
                           divNumU, divNumV);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mCylinderTriangleIndexBlocks[quality];
        rBlock.allocBuffer(
            calcIdxArrayNumCylinder_(divNumU, divNumV, cDrawType_Triangle, Quality(quality)),
            pHeap, 4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCylinder_(&mCylinderTriangleIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                                divNumU, divNumV, cDrawType_Triangle, Quality(quality));
    }

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mCylinderLineIndexBlocks[quality];
        rBlock.allocBuffer(
            calcIdxArrayNumCylinder_(divNumU, divNumV, cDrawType_Line, Quality(quality)), pHeap,
            4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCylinder_(&mCylinderLineIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                                divNumU, divNumV, cDrawType_Line, Quality(quality));
    }
}

/**
 * Creates the buffers of the capsule.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamCapsule_(u32 divNumU, u32 divNumV, u32 divNumH, sead::Heap* pHeap)
{
    mCapsuleVertexBlock.allocBuffer(calcVtxArrayNumCapsule(divNumU, divNumV, divNumH), pHeap, 8,
                                    MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mCapsuleVertexBlock, 0));
    setupVtxBufferCapsule(&mCapsuleVertexBuffer, GPUMemAddr<Vertex>(mCapsuleVertexBlock, 0),
                          divNumU, divNumV, divNumH);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mCapsuleTriangleIndexBlocks[quality];
        rBlock.allocBuffer(calcIdxArrayNumCapsule_(divNumU, divNumV, divNumH, cDrawType_Triangle,
                                                   Quality(quality)),
                           pHeap, 4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCapsule_(&mCapsuleTriangleIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                               divNumU, divNumV, divNumH, cDrawType_Triangle, Quality(quality));
    }

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mCapsuleLineIndexBlocks[quality];
        rBlock.allocBuffer(calcIdxArrayNumCapsule_(divNumU, divNumV, divNumH, cDrawType_Line,
                                                   Quality(quality)),
                           pHeap, 4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCapsule_(&mCapsuleLineIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                               divNumU, divNumV, divNumH, cDrawType_Line, Quality(quality));
    }
}

/**
 * Creates the buffers of the cone.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveShape::setUpStreamCone_(u32 divNumU, u32 divNumV, sead::Heap* pHeap)
{
    mConeVertexBlock.allocBuffer(calcVtxArrayNumCone(divNumU, divNumV), pHeap, 8,
                                 MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mConeVertexBlock, 0));
    setupVtxBufferCone(&mConeVertexBuffer, GPUMemAddr<Vertex>(mConeVertexBlock, 0), divNumU,
                       divNumV);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mConeTriangleIndexBlocks[quality];
        rBlock.allocBuffer(
            calcIdxArrayNumCone_(divNumU, divNumV, cDrawType_Triangle, Quality(quality)), pHeap,
            4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCone_(&mConeTriangleIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                            divNumU, divNumV, cDrawType_Triangle, Quality(quality));
    }

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mConeLineIndexBlocks[quality];
        rBlock.allocBuffer(calcIdxArrayNumCone_(divNumU, divNumV, cDrawType_Line, Quality(quality)),
                           pHeap, 4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamCone_(&mConeLineIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0), divNumU,
                            divNumV, cDrawType_Line, Quality(quality));
    }
}

/**
 * Creates the buffers of the torus.
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param pHeap heap used for the GPU memory
 * @param radius radius of the tube center line
 * @param tubeRadius radius of the tube
 * @param windP number of windings around the rotation axis
 * @param windQ number of windings around the interior circle
 */
void PrimitiveShape::setUpStreamTorus_(u32 divNumU, u32 divNumV, sead::Heap* pHeap, f32 radius,
                                       f32 tubeRadius, s32 windP, s32 windQ)
{
    mTorusVertexBlock.allocBuffer(calcVtxArrayNumTorus(divNumU, divNumV), pHeap, 8,
                                  MemoryAttribute(0));
    checkAddr_(GPUMemAddr<Vertex>(mTorusVertexBlock, 0));
    setupVtxBufferTorus(&mTorusVertexBuffer, GPUMemAddr<Vertex>(mTorusVertexBlock, 0), divNumU,
                        divNumV, radius, tubeRadius, windP, windQ);

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mTorusTriangleIndexBlocks[quality];
        rBlock.allocBuffer(
            calcIdxArrayNumTorus_(divNumU, divNumV, cDrawType_Triangle, Quality(quality)), pHeap,
            4, MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamTorus_(&mTorusTriangleIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                             divNumU, divNumV, cDrawType_Triangle, Quality(quality));
    }

    for (s32 quality = 0; quality < cQuality_Num; quality++)
    {
        GPUMemBlock<u16>& rBlock = mTorusLineIndexBlocks[quality];
        rBlock.allocBuffer(
            calcIdxArrayNumTorus_(divNumU, divNumV, cDrawType_Line, Quality(quality)), pHeap, 4,
            MemoryAttribute(0));
        checkAddr_(GPUMemAddr<u16>(rBlock, 0));
        setupIdxStreamTorus_(&mTorusLineIndexStreams[quality], GPUMemAddr<u16>(rBlock, 0),
                             divNumU, divNumV, cDrawType_Line, Quality(quality));
    }
}

/**
 * Sets up the position, normal and texture coordinate streams of a vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 */
void PrimitiveShape::setUpStreams_(VertexBuffer* pVertexBuffer)
{
    pVertexBuffer->setUpStream(0, VertexStreamFormat(0x22), offsetof(Vertex, mPos), false);
    pVertexBuffer->setUpStream(1, VertexStreamFormat(0x22), offsetof(Vertex, mNormal), false);
    pVertexBuffer->setUpStream(2, VertexStreamFormat(0x16), offsetof(Vertex, mTexCoord), false);
}

/**
 * Calculates the number of vertices of a circle.
 * @param divNum number of divisions of the circumference
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumCircle(u32 divNum)
{
    return divNum + 1;
}

/**
 * Calculates the number of indices of a circle.
 * @param divNum number of divisions of the circumference
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCircle(u32 divNum, DrawType drawType)
{
    return calcIdxArrayNumCircle_(divNum, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a circle for a quality level.
 * @param divNum number of divisions of the circumference
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCircle_(u32 divNum, DrawType drawType, Quality quality)
{
    switch (drawType)
    {
    case cDrawType_Triangle:
        return (divNum >> quality) * 3;
    case cDrawType_Line:
        return (divNum >> quality) * 4;
    case cDrawType_Point:
        return divNum >> quality;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a circle and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNum number of divisions of the circumference
 */
void PrimitiveShape::setupVtxBufferCircle(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                          u32 divNum)
{
    const s32 vtxNum = calcVtxArrayNumCircle(divNum);
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());

    for (u32 i = 0; i < divNum; i++)
    {
        const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNum);
        const f32 x = sead::Mathf::cos(angle) * 0.5f;
        const f32 y = sead::Mathf::sin(angle) * 0.5f;
        vertices[i].mPos.set(x, y, 0.0f);
        vertices[i].mTexCoord.set(x + 0.5f, 0.5f - y);
        vertices[i].mNormal.set(0.0f, 0.0f, 1.0f);
    }

    vertices[divNum].mPos.set(0.0f, 0.0f, 0.0f);
    vertices[divNum].mTexCoord.set(0.5f, 0.5f);
    vertices[divNum].mNormal.set(0.0f, 0.0f, 1.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a circle and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNum number of divisions of the circumference
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCircle(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                          u32 divNum, DrawType drawType)
{
    setupIdxStreamCircle_(pIndexStream, addr, divNum, drawType, cQuality_High);
}

/**
 * Writes the indices of a circle for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNum number of divisions of the circumference
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamCircle_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                           u32 divNum, DrawType drawType, Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNum; i += 1 << quality)
        {
            indices[num + 0] = divNum;
            indices[num + 1] = i;
            indices[num + 2] = (i + (1 << quality)) % divNum;
            num += 3;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 i = 0; i < divNum; i += 1 << quality)
        {
            indices[num + 0] = divNum;
            indices[num + 1] = i;
            indices[num + 2] = i;
            indices[num + 3] = (i + (1 << quality)) % divNum;
            num += 4;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    case cDrawType_Point:
        for (u32 i = 0; i < divNum; i += 1 << quality)
        {
            indices[num] = i;
            num++;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a circle and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNum number of divisions of the circumference
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCircle(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                          u32 divNum, DrawType drawType)
{
    setupIdxStreamCircle_(pIndexStream, addr, divNum, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a sphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumSphere(u32 divNumU, u32 divNumV)
{
    return divNumU * divNumV + 2;
}

/**
 * Calculates the number of indices of a sphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumSphere(u32 divNumU, u32 divNumV, DrawType drawType)
{
    return calcIdxArrayNumSphere_(divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a sphere for a quality level.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumSphere_(u32 divNumU, u32 divNumV, DrawType drawType,
                                           Quality quality)
{
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 6;
        }

        for (u32 j = 0; j < divNumV - 1; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 6;
            }
        }

        return num;
    case cDrawType_Line:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 6;
        }

        for (u32 j = 0; j < divNumV - 1; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 4;
            }
        }

        return num;
    case cDrawType_Point:
        for (u32 j = 0; j < divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 2;
            }
        }

        return num + 2;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a sphere and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 */
void PrimitiveShape::setupVtxBufferSphere(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                          u32 divNumU, u32 divNumV)
{
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    const s32 vtxNum = calcVtxArrayNumSphere(divNumU, divNumV);

    for (u32 j = 0; j < divNumV; j++)
    {
        const f32 latitude = (f32(j + 1) / (f32(divNumV) + 1.0f) - 0.5f) * sead::Mathf::pi();
        const f32 y = sead::Mathf::sin(latitude) * 0.5f;
        const f32 radius = sead::Mathf::cos(latitude) * 0.5f;

        for (u32 i = 0; i < divNumU; i++)
        {
            const s32 index = j * divNumU + i;
            const f32 longitude = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = radius * sead::Mathf::cos(longitude);
            const f32 z = radius * sead::Mathf::sin(longitude);
            Vertex& rVertex = vertices[index];
            rVertex.mPos.set(x, y, z);
            rVertex.mTexCoord.set(x + 0.5f, z + 0.5f);
            rVertex.mNormal.set(x * 2.0f, y * 2.0f, z * 2.0f);
        }
    }

    vertices[s32(divNumU * divNumV)].mPos.set(0.0f, -0.5f, 0.0f);
    vertices[s32(divNumU * divNumV)].mTexCoord.set(0.5f, 0.5f);
    vertices[s32(divNumU * divNumV)].mNormal.set(0.0f, -1.0f, 0.0f);
    vertices[s32(divNumU * divNumV + 1)].mPos.set(0.0f, 0.5f, 0.0f);
    vertices[s32(divNumU * divNumV + 1)].mTexCoord.set(0.5f, 0.5f);
    vertices[s32(divNumU * divNumV + 1)].mNormal.set(0.0f, 1.0f, 0.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a sphere and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamSphere(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                          u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamSphere_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Writes the indices of a sphere for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamSphere_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                           u32 divNumU, u32 divNumV, DrawType drawType,
                                           Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = divNumU * divNumV;
            indices[num++] = i;
            indices[num++] = (i + (1 << quality)) % divNumU;
        }

        for (u32 j = 0; j < divNumV - 1; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(divNumV - 1), s32(j + (1 << quality))) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = row1 + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + i;
                indices[num++] = row1 + (i + (1 << quality)) % divNumU;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = divNumU * divNumV + 1;
            indices[num++] = (divNumV - 1) * divNumU + (i + (1 << quality)) % divNumU;
            indices[num++] = (divNumV - 1) * divNumU + i;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = divNumU * divNumV;
            indices[num++] = i;
            indices[num++] = divNumU * divNumV + 1;
            indices[num++] = (divNumV - 1) * divNumU + (i + (1 << quality)) % divNumU;
            indices[num++] = (divNumV - 1) * divNumU + (i + (1 << quality)) % divNumU;
            indices[num++] = (divNumV - 1) * divNumU + i;
        }

        for (u32 j = 0; j < divNumV - 1; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(j + (1 << quality)), s32(divNumV - 1)) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = j * divNumU + i;
                indices[num++] = row1 + i;
            }
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    case cDrawType_Point:
        for (u32 j = 0; j < divNumV; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(j + (1 << quality)), s32(divNumV - 1)) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + i;
            }
        }

        indices[num++] = divNumU * divNumV;
        indices[num++] = divNumU * divNumV + 1;
        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a sphere and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings between the poles
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamSphere(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                          u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamSphere_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a hemisphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumHemisphere(u32 divNumU, u32 divNumV)
{
    return (divNumV / 2 + 1) * divNumU + 1;
}

/**
 * Calculates the number of indices of a hemisphere.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumHemisphere(u32 divNumU, u32 divNumV, DrawType drawType)
{
    return calcIdxArrayNumHemisphere_(divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a hemisphere for a quality level.
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumHemisphere_(u32 divNumU, u32 divNumV, DrawType drawType,
                                               Quality quality)
{
    const u32 halfNumV = divNumV / 2;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 3;
        }

        for (u32 j = 0; j < halfNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 6;
            }
        }

        return num;
    case cDrawType_Line:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 4;
        }

        for (u32 j = 0; j < halfNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 4;
            }
        }

        return num;
    case cDrawType_Point:
        for (u32 j = 0; j <= halfNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 2;
            }
        }

        return num + 1;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a hemisphere and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 */
void PrimitiveShape::setupVtxBufferHemisphere(VertexBuffer* pVertexBuffer,
                                              GPUMemAddr<Vertex> addr, u32 divNumU, u32 divNumV)
{
    const f32 step = sead::Mathf::pi() / (f32(divNumV) + 1.0f);
    const u32 halfNumV = divNumV / 2;
    const u32 ringNum = halfNumV + 1;
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    const s32 vtxNum = calcVtxArrayNumHemisphere(divNumU, divNumV);

    for (u32 j = 0; j <= halfNumV; j++)
    {
        const f32 latitude = std::fmax(sead::Mathf::piHalf() - step * f32(j + 1), 0.0f);
        const f32 y = sead::Mathf::sin(latitude) * 0.5f;
        const f32 radius = sead::Mathf::cos(latitude) * 0.5f;

        for (u32 i = 0; i < divNumU; i++)
        {
            const s32 index = j * divNumU + i;
            const f32 longitude = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = radius * sead::Mathf::cos(longitude);
            const f32 z = radius * sead::Mathf::sin(longitude);
            Vertex& rVertex = vertices[index];
            rVertex.mPos.set(x, y, z);
            rVertex.mTexCoord.set(x + 0.5f, z + 0.5f);
            rVertex.mNormal.set(x * 2.0f, y * 2.0f, z * 2.0f);
        }
    }

    vertices[s32(ringNum * divNumU)].mPos.set(0.0f, 0.5f, 0.0f);
    vertices[s32(ringNum * divNumU)].mTexCoord.set(0.5f, 0.5f);
    vertices[s32(ringNum * divNumU)].mNormal.set(0.0f, 1.0f, 0.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), s32(vtxNum * sizeof(Vertex)));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a hemisphere and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamHemisphere(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                              u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamHemisphere_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Writes the indices of a hemisphere for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamHemisphere_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                               u32 divNumU, u32 divNumV, DrawType drawType,
                                               Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    const u32 halfNumV = divNumV / 2;
    const u32 ringNum = halfNumV + 1;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = ringNum * divNumU;
            indices[num++] = (i + (1 << quality)) % divNumU;
            indices[num++] = i;
        }

        for (u32 j = 0; j < halfNumV; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(halfNumV), s32(j + (1 << quality))) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + i;
                indices[num++] = row1 + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + (i + (1 << quality)) % divNumU;
            }
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = ringNum * divNumU;
            indices[num++] = i;
            indices[num++] = halfNumV * divNumU + (i + (1 << quality)) % divNumU;
            indices[num++] = halfNumV * divNumU + i;
        }

        for (u32 j = 0; j < halfNumV; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(j + (1 << quality)), s32(halfNumV)) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = j * divNumU + i;
                indices[num++] = row1 + i;
            }
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    case cDrawType_Point:
        for (u32 j = 0; j <= halfNumV; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::min(s32(j + (1 << quality)), s32(halfNumV)) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + i;
            }
        }

        indices[num++] = ringNum * divNumU;
        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a hemisphere and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the vertical axis
 * @param divNumV number of rings of the full sphere
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamHemisphere(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                              u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamHemisphere_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a cylinder.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumCylinder(u32 divNumU, u32 divNumV)
{
    return (divNumV + 3) * divNumU + 2;
}

/**
 * Calculates the number of indices of a cylinder.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCylinder(u32 divNumU, u32 divNumV, DrawType drawType)
{
    return calcIdxArrayNumCylinder_(divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a cylinder for a quality level.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCylinder_(u32 divNumU, u32 divNumV, DrawType drawType,
                                             Quality quality)
{
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 3;
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 6;
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 3;
        }

        return num;
    case cDrawType_Line:
        for (u32 j = 1; j <= divNumV + 1; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 2;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 6;
        }

        return num;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a cylinder and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 */
void PrimitiveShape::setupVtxBufferCylinder(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                            u32 divNumU, u32 divNumV)
{
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    const s32 vtxNum = calcVtxArrayNumCylinder(divNumU, divNumV);
    s32 index = 0;

    vertices[index].mPos.set(0.0f, 0.5f, 0.0f);
    vertices[index].mTexCoord.set(0.5f, 0.5f);
    vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
    index++;

    for (u32 i = 0; i < divNumU; i++)
    {
        const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
        const f32 x = sead::Mathf::cos(angle) * 0.5f;
        const f32 z = sead::Mathf::sin(angle) * 0.5f;
        vertices[index].mPos.set(x, 0.5f, z);
        vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
        vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
        index++;
    }

    for (u32 j = 0; j <= divNumV; j++)
    {
        const f32 y = 0.5f - f32(j) / f32(divNumV);

        for (u32 i = 0; i < divNumU; i++)
        {
            const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = sead::Mathf::cos(angle) * 0.5f;
            const f32 z = sead::Mathf::sin(angle) * 0.5f;
            vertices[index].mPos.set(x, y, z);
            vertices[index].mTexCoord.set(x + 0.5f, y + 0.5f);
            vertices[index].mNormal.set(x * 2.0f, 0.0f, z * 2.0f);
            index++;
        }
    }

    for (u32 i = 0; i < divNumU; i++)
    {
        const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
        const f32 x = sead::Mathf::cos(angle) * 0.5f;
        const f32 z = sead::Mathf::sin(angle) * 0.5f;
        vertices[index].mPos.set(x, -0.5f, z);
        vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
        vertices[index].mNormal.set(0.0f, -1.0f, 0.0f);
        index++;
    }

    vertices[index].mPos.set(0.0f, -0.5f, 0.0f);
    vertices[index].mTexCoord.set(0.5f, 0.5f);
    vertices[index].mNormal.set(0.0f, -1.0f, 0.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a cylinder and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCylinder(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                            u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamCylinder_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Writes the indices of a cylinder for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamCylinder_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                             u32 divNumU, u32 divNumV, DrawType drawType,
                                             Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    const u32 bottomRingEnd = (divNumV + 3) * divNumU;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = i + 1;
            indices[num++] = 0;
            indices[num++] = (i + (1 << quality)) % divNumU + 1;
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = divNumU + i + 1;
            indices[num++] = divNumU + 1 + (i + (1 << quality)) % divNumU;
            indices[num++] = (divNumV + 1) * divNumU + 1 + i;
            indices[num++] = (divNumV + 1) * divNumU + 1 + i;
            indices[num++] = divNumU + 1 + (i + (1 << quality)) % divNumU;
            indices[num++] = (divNumV + 1) * divNumU + 1 + (i + (1 << quality)) % divNumU;
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = (bottomRingEnd + 1) - (1 << quality) - i;
            indices[num++] = (bottomRingEnd + 1);
            indices[num++] = (bottomRingEnd + 1) - (1 << quality) - (i + (1 << quality)) % divNumU;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 j = 1; j <= divNumV + 1; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + 1 + i;
                indices[num++] = j * divNumU + 1 + (i + (1 << quality)) % divNumU;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = 0;
            indices[num++] = i + 1;
            indices[num++] = divNumU + 1 + i;
            indices[num++] = (divNumV + 1) * divNumU + 1 + i;
            indices[num++] = (divNumV + 2) * divNumU + 1 + i;
            indices[num++] = (bottomRingEnd + 1);
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a cylinder and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCylinder(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                            u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamCylinder_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a capsule.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumCapsule(u32 divNumU, u32 divNumV, u32 divNumH)
{
    return (divNumV + divNumH * 2 - 1) * divNumU + 2;
}

/**
 * Calculates the number of indices of a capsule.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCapsule(u32 divNumU, u32 divNumV, u32 divNumH,
                                           DrawType drawType)
{
    return calcIdxArrayNumCapsule_(divNumU, divNumV, divNumH, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a capsule for a quality level.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCapsule_(u32 divNumU, u32 divNumV, u32 divNumH,
                                            DrawType drawType, Quality quality)
{
    const u32 step = 1 << quality;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += step)
        {
            num += 3;
        }

        for (u32 j = step; j < divNumH; j += step)
        {
            for (u32 i = 0; i < divNumU; i += step)
            {
                num += 6;
            }
        }

        for (u32 i = 0; i < divNumU; i += step)
        {
            for (u32 j = 0; j <= divNumV; j += step)
            {
                num += 6;
            }
        }

        for (u32 j = step; j < divNumH - step; j += step)
        {
            for (u32 i = 0; i < divNumU; i += step)
            {
                num += 6;
            }
        }

        for (u32 i = 0; i < divNumU; i += step)
        {
            num += 3;
        }

        return num;
    case cDrawType_Line:
        for (u32 i = 0; i < divNumU; i += step)
        {
            num += 2;
        }

        for (u32 j = step; j < divNumH; j += step)
        {
            for (u32 i = 0; i < divNumU; i += step)
            {
                num += 4;
            }
        }

        for (u32 i = 0; i < divNumU; i += step)
        {
            for (u32 j = 0; j <= divNumV; j += step)
            {
                num += 4;
            }
        }

        for (u32 j = step; j < divNumH; j += step)
        {
            for (u32 i = 0; i < divNumU; i += step)
            {
                num += 4;
            }
        }

        return num;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a capsule and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 */
void PrimitiveShape::setupVtxBufferCapsule(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                           u32 divNumU, u32 divNumV, u32 divNumH)
{
    const u32 ringNum = divNumH * 2;
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    const s32 vtxNum = calcVtxArrayNumCapsule(divNumU, divNumV, divNumH);
    s32 index = 0;

    vertices[index].mPos.set(0.0f, 1.0f, 0.0f);
    vertices[index].mTexCoord.set(0.5f, 0.5f);
    vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
    index++;

    for (u32 j = 1; j < divNumH; j++)
    {
        const f32 latitude = (f32(j) / (f32(divNumH) + f32(divNumH)) - 0.5f) * sead::Mathf::pi();
        const f32 y = sead::Mathf::sin(latitude) * -0.5f;
        const f32 radius = sead::Mathf::cos(latitude) * 0.5f;

        for (u32 i = 0; i < divNumU; i++)
        {
            const f32 longitude = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = radius * sead::Mathf::cos(longitude);
            const f32 z = radius * sead::Mathf::sin(longitude);
            vertices[index].mPos.set(x, y + 0.5f, z);
            vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
            vertices[index].mNormal.set(x * 2.0f, y * 2.0f, z * 2.0f);
            index++;
        }
    }

    for (u32 j = 0; j <= divNumV; j++)
    {
        const f32 y = 0.5f - f32(j) / f32(divNumV);

        for (u32 i = 0; i < divNumU; i++)
        {
            const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = sead::Mathf::cos(angle) * 0.5f;
            const f32 z = sead::Mathf::sin(angle) * 0.5f;
            vertices[index].mPos.set(x, y, z);
            vertices[index].mTexCoord.set(x + 0.5f, y + 0.5f);
            vertices[index].mNormal.set(x * 2.0f, 0.0f, z * 2.0f);
            index++;
        }
    }

    for (u32 j = divNumH + 1; j < ringNum; j++)
    {
        const f32 latitude = (f32(j) / (f32(divNumH) + f32(divNumH)) - 0.5f) * sead::Mathf::pi();
        const f32 y = sead::Mathf::sin(latitude) * -0.5f;
        const f32 radius = sead::Mathf::cos(latitude) * 0.5f;

        for (u32 i = 0; i < divNumU; i++)
        {
            const f32 longitude = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            const f32 x = radius * sead::Mathf::cos(longitude);
            const f32 z = radius * sead::Mathf::sin(longitude);
            vertices[index].mPos.set(x, y - 0.5f, z);
            vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
            vertices[index].mNormal.set(x * 2.0f, y * 2.0f, z * 2.0f);
            index++;
        }
    }

    vertices[index].mPos.set(0.0f, -1.0f, 0.0f);
    vertices[index].mTexCoord.set(0.5f, 0.5f);
    vertices[index].mNormal.set(0.0f, -1.0f, 0.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a capsule and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCapsule(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                           u32 divNumU, u32 divNumV, u32 divNumH,
                                           DrawType drawType)
{
    setupIdxStreamCapsule_(pIndexStream, addr, divNumU, divNumV, divNumH, drawType,
                           cQuality_High);
}

/**
 * Writes the indices of a capsule for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamCapsule_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                            u32 divNumU, u32 divNumV, u32 divNumH,
                                            DrawType drawType, Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    const u32 step = 1 << quality;
    const u32 topRing = (divNumU << quality) - divNumU + 1;
    const u32 hemisphereSize = (divNumH - step) * divNumU;
    const u32 cylinderSize = (divNumV + step) * divNumU;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
    {
        for (u32 i = 0; i < divNumU; i += step)
        {
            indices[num++] = topRing + i;
            indices[num++] = 0;
            indices[num++] = topRing + (i + step) % divNumU;
        }

        for (u32 j = step; j < divNumH; j += step)
        {
            const u32 row0 = (j - step) * divNumU + topRing;
            const u32 row1 = j * divNumU + topRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 sideRing = topRing + hemisphereSize;

        for (u32 j = 0; j <= divNumV; j += step)
        {
            const u32 row0 = j * divNumU + sideRing;
            const u32 row1 = (j + step) * divNumU + sideRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 bottomRing = sideRing + cylinderSize;

        for (u32 j = step; j < divNumH - step; j += step)
        {
            const u32 row0 = (j - step) * divNumU + bottomRing;
            const u32 row1 = j * divNumU + bottomRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row1 + (i + step) % divNumU;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 bottomPole = cylinderSize + topRing + hemisphereSize * 2;

        for (u32 i = 0; i < divNumU; i += step)
        {
            indices[num++] = bottomPole;
            indices[num++] = bottomPole - divNumU * step + i;
            indices[num++] = bottomPole - divNumU * step + (i + step) % divNumU;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    }
    case cDrawType_Line:
    {
        for (u32 i = 0; i < divNumU; i += step)
        {
            indices[num++] = topRing + i;
            indices[num++] = 0;
        }

        for (u32 j = step; j < divNumH; j += step)
        {
            const u32 row0 = (j - step) * divNumU + topRing;
            const u32 row1 = j * divNumU + topRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 sideRing = topRing + hemisphereSize;

        for (u32 j = 0; j <= divNumV; j += step)
        {
            const u32 row0 = j * divNumU + sideRing;
            const u32 row1 = (j + step) * divNumU + sideRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 bottomRing = sideRing + cylinderSize;

        for (u32 j = step; j < divNumH - step; j += step)
        {
            const u32 row0 = (j - step) * divNumU + bottomRing;
            const u32 row1 = j * divNumU + bottomRing;

            for (u32 i = 0; i < divNumU; i += step)
            {
                indices[num++] = row1 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + i;
                indices[num++] = row0 + (i + step) % divNumU;
            }
        }

        const u32 bottomPole = cylinderSize + topRing + hemisphereSize * 2;

        for (u32 i = 0; i < divNumU; i += step)
        {
            indices[num++] = bottomPole;
            indices[num++] = bottomPole - divNumU * step + i;
            indices[num++] = bottomPole - divNumU * step + i;
            indices[num++] = bottomPole - divNumU * step + (i + step) % divNumU;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    }
    default:
        break;
    }
}

/**
 * Writes the indices of a capsule and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions of the cylinder part
 * @param divNumH number of rings of each hemisphere
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCapsule(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                           u32 divNumU, u32 divNumV, u32 divNumH,
                                           DrawType drawType)
{
    setupIdxStreamCapsule_(pIndexStream, addr, divNumU, divNumV, divNumH, drawType,
                           cQuality_High);
}

/**
 * Calculates the number of vertices of a cone.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumCone(u32 divNumU, u32 divNumV)
{
    return (divNumV + 2) * divNumU + 1;
}

/**
 * Calculates the number of indices of a cone.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCone(u32 divNumU, u32 divNumV, DrawType drawType)
{
    return calcIdxArrayNumCone_(divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a cone for a quality level.
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumCone_(u32 divNumU, u32 divNumV, DrawType drawType,
                                         Quality quality)
{
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 3;
        }

        for (u32 j = 1; j < divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 6;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 3;
        }

        return num;
    case cDrawType_Line:
        for (u32 j = 1; j <= divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                num += 2;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            num += 4;
        }

        return num;
    default:
        return 0;
    }
}

/**
 * Writes the vertices of a cone and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 */
void PrimitiveShape::setupVtxBufferCone(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                        u32 divNumU, u32 divNumV)
{
    const s32 vtxNum = calcVtxArrayNumCone(divNumU, divNumV);
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    s32 index = 0;

    for (u32 j = 0; j <= divNumV; j++)
    {
        const f32 y = 0.5f - f32(j) / f32(divNumV);
        const f32 radius = (0.5f - y) * 0.5f;

        for (u32 i = 0; i < divNumU; i++)
        {
            f32 angle;

            if (j == 0)
            {
                angle = (f32(i) + 0.5f) * sead::Mathf::pi2() / f32(divNumU);
            }
            else
            {
                angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            }

            const f32 x = radius * sead::Mathf::cos(angle);
            const f32 z = radius * sead::Mathf::sin(angle);
            vertices[index].mPos.set(x, y, z);
            vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
            vertices[index].mNormal.set(x, 0.5f, z);
            vertices[index].mNormal.normalize();
            index++;
        }
    }

    for (u32 i = 0; i < divNumU; i++)
    {
        const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
        const f32 x = sead::Mathf::cos(angle) * 0.5f;
        const f32 z = sead::Mathf::sin(angle) * 0.5f;
        vertices[index].mPos.set(x, -0.5f, z);
        vertices[index].mTexCoord.set(x + 0.5f, z + 0.5f);
        vertices[index].mNormal.set(0.0f, -1.0f, 0.0f);
        index++;
    }

    vertices[index].mPos.set(0.0f, -0.5f, 0.0f);
    vertices[index].mTexCoord.set(0.5f, 0.5f);
    vertices[index].mNormal.set(0.0f, -1.0f, 0.0f);

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Writes the indices of a cone and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCone(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                        u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamCone_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Writes the indices of a cone for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamCone_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                         u32 divNumU, u32 divNumV, DrawType drawType,
                                         Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    const u32 bottomCenter = (divNumV + 2) * divNumU;
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = divNumU + i;
            indices[num++] = i;
            indices[num++] = divNumU + (i + (1 << quality)) % divNumU;
        }

        for (u32 j = 1; j < divNumV; j += 1 << quality)
        {
            const u32 row1 = sead::Mathi::clampMax(s32(j + (1 << quality)), s32(divNumV)) * divNumU;

            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = row1 + i;
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = row1 + (i + (1 << quality)) % divNumU;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = bottomCenter - (1 << quality) - i;
            indices[num++] = bottomCenter;
            indices[num++] = bottomCenter - (1 << quality) - (i + (1 << quality)) % divNumU;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 j = 1 << quality; j <= divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
            }
        }

        for (u32 i = 0; i < divNumU; i += 1 << quality)
        {
            indices[num++] = i;
            indices[num++] = divNumV * divNumU + i;
            indices[num++] = (divNumV + 1) * divNumU + i;
            indices[num++] = bottomCenter;
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a cone and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the axis
 * @param divNumV number of divisions along the axis
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamCone(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                        u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamCone_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a torus.
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumTorus(u32 divNumU, u32 divNumV)
{
    return (divNumV + 1) * divNumU;
}

/**
 * Calculates the number of indices of a torus.
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param drawType primitive type
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumTorus(u32 divNumU, u32 divNumV, DrawType drawType)
{
    return calcIdxArrayNumTorus_(divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of indices of a torus for a quality level.
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param drawType primitive type
 * @param quality quality level
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumTorus_(u32 divNumU, u32 divNumV, DrawType drawType,
                                          Quality quality)
{
    return (divNumU >> quality) * (divNumV >> quality) * (drawType == cDrawType_Triangle ? 6 : 4);
}

void PrimitiveShape::setupVtxBufferTorus(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                         u32 divNumU, u32 divNumV, f32 radius, f32 tubeRadius,
                                         s32 windP, s32 windQ)
{
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    const s32 vtxNum = calcVtxArrayNumTorus(divNumU, divNumV);

    sead::Vector3f max = -sead::Vector3f::ones * radius;
    sead::Vector3f min = sead::Vector3f::ones * radius;
    const f32 step = 1.0f / f32(divNumV);

    for (u32 i = 0; i < divNumV; i++)
    {
        const sead::Vector3f center = calcTorusCircleCenter_(windP, windQ, step * f32(i), radius);
        max.x = sead::Mathf::max(max.x, center.x);
        max.y = sead::Mathf::max(max.y, center.y);
        max.z = sead::Mathf::max(max.z, center.z);
        min.x = sead::Mathf::min(min.x, center.x);
        min.y = sead::Mathf::min(min.y, center.y);
        min.z = sead::Mathf::min(min.z, center.z);
    }

    const sead::Vector3f mid = (min + max) * 0.5f;
    sead::Vector3f prev = calcTorusCircleCenter_(windP, windQ, 1.0f - step, radius) - mid;
    sead::Vector3f dir = prev;
    s32 index = 0;

    for (u32 j = 0; j < divNumV + 1; j++)
    {
        dir.normalize();
        const sead::Vector3f center =
            calcTorusCircleCenter_(windP, windQ, step * f32(j), radius) - mid;
        sead::Vector3f tangent = center - prev;
        tangent.normalize();
        sead::Vector3f binormal;
        binormal.setCross(tangent, dir);
        binormal.normalize();
        dir.setCross(tangent, binormal);
        dir = -dir;
        prev = center;

        sead::Matrix34f mtx;
        mtx.setBase(0, dir);
        mtx.setBase(1, binormal);
        mtx.setBase(2, tangent);
        mtx.setBase(3, center);

        for (u32 i = 0; i < divNumU; i++)
        {
            const f32 angle = f32(i) * sead::Mathf::pi2() / f32(divNumU);
            f32 sin;
            f32 cos;
            sead::Mathf::sinCosIdx(&sin, &cos, rad2idx_(angle));
            Vertex& rVertex = vertices[index];
            rVertex.mPos.setMul(mtx, sead::Vector3f(sin * tubeRadius, cos * tubeRadius, 0.0f));
            rVertex.mNormal.setRotated(mtx, sead::Vector3f(sin, cos, 0.0f));
            rVertex.mTexCoord.set(rVertex.mPos.x + 0.5f, rVertex.mPos.y + 0.5f);
            index++;
        }
    }

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Calculates a point on the torus knot center line.
 * @param windP number of windings around the rotation axis
 * @param windQ number of windings around the interior circle
 * @param t position along the center line in [0, 1)
 * @param radius radius of the center line
 * @return center line point
 */
sead::Vector3f PrimitiveShape::calcTorusCircleCenter_(s32 windP, s32 windQ, f32 t, f32 radius)
{
    const f32 angleP = t * f32(windP) * sead::Mathf::pi2();
    const f32 sinP = sead::Mathf::sin(angleP);
    const f32 cosP = sead::Mathf::cos(angleP);
    const f32 angleQ = t * f32(windQ) * sead::Mathf::pi2();
    const f32 sinQ = sead::Mathf::sin(angleQ);
    const f32 cosQ = sead::Mathf::cos(angleQ);
    const f32 scale = cosP * 0.5f + 1.0f;
    const sead::Vector3f center(scale * cosQ - 1.0f, sinQ * scale, sinP * 0.5f);
    return center * radius * (2.0f / 3.0f);
}

/**
 * Writes the indices of a torus and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamTorus(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                         u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamTorus_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Writes the indices of a torus for a quality level and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param drawType primitive type
 * @param quality quality level
 */
template <typename T>
void PrimitiveShape::setupIdxStreamTorus_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                          u32 divNumU, u32 divNumV, DrawType drawType,
                                          Quality quality)
{
    T* indices = static_cast<T*>(addr.getPtr());
    s32 num = 0;

    switch (drawType)
    {
    case cDrawType_Triangle:
        for (u32 j = 0; j < divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = j * divNumU + i;
                indices[num++] = (j + (1 << quality)) * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = (j + (1 << quality)) * divNumU + i;
                indices[num++] = (j + (1 << quality)) * divNumU + (i + (1 << quality)) % divNumU;
            }
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
        break;
    case cDrawType_Line:
        for (u32 j = 0; j < divNumV; j += 1 << quality)
        {
            for (u32 i = 0; i < divNumU; i += 1 << quality)
            {
                indices[num++] = j * divNumU + i;
                indices[num++] = j * divNumU + (i + (1 << quality)) % divNumU;
                indices[num++] = j * divNumU + i;
                indices[num++] = (j + (1 << quality)) * divNumU + i;
            }
        }

        pIndexStream->setUpStream(addr, num);
        pIndexStream->setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
        break;
    default:
        break;
    }
}

/**
 * Writes the indices of a torus and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumU number of divisions around the tube
 * @param divNumV number of divisions along the tube
 * @param drawType primitive type
 */
void PrimitiveShape::setupIdxStreamTorus(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                         u32 divNumU, u32 divNumV, DrawType drawType)
{
    setupIdxStreamTorus_(pIndexStream, addr, divNumU, divNumV, drawType, cQuality_High);
}

/**
 * Calculates the number of vertices of a grid quad.
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumGridQuad(u32 divNumX, u32 divNumY)
{
    return (divNumX + 1) * 2 + (divNumY + 1) * 2;
}

/**
 * Calculates the number of indices of a grid quad.
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumGridQuad(u32 divNumX, u32 divNumY)
{
    return (divNumX + 1) * 2 + (divNumY + 1) * 2;
}

/**
 * Writes the line indices of a grid quad and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 */
void PrimitiveShape::setupIdxStreamGridQuad(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                            u32 divNumX, u32 divNumY)
{
    setupIdxStreamGrid_(pIndexStream, addr, calcIdxArrayNumGridQuad(divNumX, divNumY));
}

/**
 * Writes the line indices of a grid quad and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 */
void PrimitiveShape::setupIdxStreamGridQuad(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                            u32 divNumX, u32 divNumY)
{
    setupIdxStreamGrid_(pIndexStream, addr, calcIdxArrayNumGridQuad(divNumX, divNumY));
}

/**
 * Writes the vertices of a grid quad and sets up the vertex buffer.
 * @param pVertexBuffer vertex buffer to set up
 * @param addr vertex memory
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 */
void PrimitiveShape::setupVtxBufferGridQuad(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                            u32 divNumX, u32 divNumY)
{
    const s32 vtxNum = calcVtxArrayNumGridQuad(divNumX, divNumY);
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());
    s32 index = 0;

    const f32 stepX = 1.0f / f32(divNumX);
    const f32 stepY = 1.0f / f32(divNumY);

    for (u32 i = 0; i <= divNumX; i++)
    {
        const f32 t = stepX * f32(i);
        const f32 x = t - 0.5f;
        vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
        vertices[index].mTexCoord.set(t, 0.0f);
        vertices[index].mPos.set(x, 0.0f, -0.5f);
        vertices[index + 1].mNormal.set(0.0f, 1.0f, 0.0f);
        vertices[index + 1].mTexCoord.set(t, 1.0f);
        vertices[index + 1].mPos.set(x, 0.0f, 0.5f);
        index += 2;
    }

    for (u32 i = 0; i <= divNumY; i++)
    {
        const f32 t = stepY * f32(i);
        const f32 z = t - 0.5f;
        vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
        vertices[index].mTexCoord.set(0.0f, t);
        vertices[index].mPos.set(-0.5f, 0.0f, z);
        vertices[index + 1].mNormal.set(0.0f, 1.0f, 0.0f);
        vertices[index + 1].mTexCoord.set(1.0f, t);
        vertices[index + 1].mPos.set(0.5f, 0.0f, z);
        index += 2;
    }

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Calculates the number of vertices of a grid cube.
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @param divNumZ number of divisions along z
 * @return number of vertices
 */
u32 PrimitiveShape::calcVtxArrayNumGridCube(u32 divNumX, u32 divNumY, u32 divNumZ)
{
    return ((divNumX + 1) * (divNumY + 1) + (divNumY + 1) * (divNumZ + 1) +
            (divNumZ + 1) * (divNumX + 1)) *
           2;
}

/**
 * Calculates the number of indices of a grid cube.
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @param divNumZ number of divisions along z
 * @return number of indices
 */
u32 PrimitiveShape::calcIdxArrayNumGridCube(u32 divNumX, u32 divNumY, u32 divNumZ)
{
    return ((divNumX + 1) * (divNumY + 1) + (divNumY + 1) * (divNumZ + 1) +
            (divNumZ + 1) * (divNumX + 1)) *
           2;
}

/**
 * Writes the line indices of a grid cube and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @param divNumZ number of divisions along z
 */
void PrimitiveShape::setupIdxStreamGridCube(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                            u32 divNumX, u32 divNumY, u32 divNumZ)
{
    setupIdxStreamGrid_(pIndexStream, addr, calcIdxArrayNumGridCube(divNumX, divNumY, divNumZ));
}

/**
 * Writes the line indices of a grid cube and sets up the index stream.
 * @param pIndexStream index stream to set up
 * @param addr index memory
 * @param divNumX number of divisions along x
 * @param divNumY number of divisions along y
 * @param divNumZ number of divisions along z
 */
void PrimitiveShape::setupIdxStreamGridCube(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                            u32 divNumX, u32 divNumY, u32 divNumZ)
{
    setupIdxStreamGrid_(pIndexStream, addr, calcIdxArrayNumGridCube(divNumX, divNumY, divNumZ));
}

void PrimitiveShape::setupVtxBufferGridCube(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                            u32 divNumX, u32 divNumY, u32 divNumZ)
{
    const s32 vtxNum = calcVtxArrayNumGridCube(divNumX, divNumY, divNumZ);
    Vertex* vertices = static_cast<Vertex*>(addr.getPtr());

    const f32 stepX = 1.0f / f32(divNumX);
    const f32 stepY = 1.0f / f32(divNumY);
    const f32 stepZ = 1.0f / f32(divNumZ);
    s32 index = 0;

    for (u32 i = 0; i <= divNumX; i++)
    {
        const f32 x = stepX * f32(i);

        for (u32 j = 0; j <= divNumY; j++)
        {
            const f32 y = stepY * f32(j);
            vertices[index].mPos.set(x, y, 0.0f);
            vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index].mTexCoord.set(x, 0.0f);
            vertices[index].mPos -= sead::Vector3f::ones * 0.5f;
            vertices[index + 1].mPos.set(x, y, 1.0f);
            vertices[index + 1].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index + 1].mTexCoord.set(x, 1.0f);
            vertices[index + 1].mPos -= sead::Vector3f::ones * 0.5f;
            index += 2;
        }
    }

    for (u32 i = 0; i <= divNumY; i++)
    {
        const f32 y = stepY * f32(i);

        for (u32 j = 0; j <= divNumZ; j++)
        {
            const f32 z = stepZ * f32(j);
            vertices[index].mPos.set(0.0f, y, z);
            vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index].mTexCoord.set(0.0f, z);
            vertices[index].mPos -= sead::Vector3f::ones * 0.5f;
            vertices[index + 1].mPos.set(1.0f, y, z);
            vertices[index + 1].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index + 1].mTexCoord.set(1.0f, z);
            vertices[index + 1].mPos -= sead::Vector3f::ones * 0.5f;
            index += 2;
        }
    }

    for (u32 i = 0; i <= divNumZ; i++)
    {
        const f32 z = stepZ * f32(i);

        for (u32 j = 0; j <= divNumX; j++)
        {
            const f32 x = stepX * f32(j);
            vertices[index].mPos.set(x, 0.0f, z);
            vertices[index].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index].mTexCoord.set(x, z);
            vertices[index].mPos -= sead::Vector3f::ones * 0.5f;
            vertices[index + 1].mPos.set(x, 1.0f, z);
            vertices[index + 1].mNormal.set(0.0f, 1.0f, 0.0f);
            vertices[index + 1].mTexCoord.set(x, z);
            vertices[index + 1].mPos -= sead::Vector3f::ones * 0.5f;
            index += 2;
        }
    }

    pVertexBuffer->setUpBuffer(addr, sizeof(Vertex), vtxNum * sizeof(Vertex));
    setUpStreams_(pVertexBuffer);
}

/**
 * Does nothing.
 * @param pContext host IO context
 */
void PrimitiveShape::genMessage(sead::hostio::Context* pContext) {}

/**
 * Does nothing.
 * @param pEvent property event
 */
void PrimitiveShape::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::utl
