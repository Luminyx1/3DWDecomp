#pragma once

#include <container/seadSafeArray.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglIndexStream.h"
#include "common/aglVertexBuffer.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl::utl {

class PrimitiveShape : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(PrimitiveShape)

    PrimitiveShape();
    virtual ~PrimitiveShape();

public:
    struct Vertex {
        sead::Vector3f mPos;
        sead::Vector3f mNormal;
        sead::Vector2f mTexCoord;
    };
    static_assert(sizeof(Vertex) == 0x20);

    enum DrawType
    {
        cDrawType_Triangle,
        cDrawType_Line,
        cDrawType_Point,
        cDrawType_Num
    };

    enum Quality
    {
        cQuality_High,
        cQuality_Middle,
        cQuality_Low,
        cQuality_Num
    };


    void initialize(sead::Heap* pHeap);

    const VertexBuffer& getQuadVertexBuffer() const { return mQuadVertexBuffer; }
    const IndexStream& getQuadIndexStream(s32 index = 0) const
    {
        return index == 0 ? mQuadTriangleIndexStream : mQuadLineIndexStream;
    }
    const VertexBuffer& getQuadTriangleVertexBuffer() const { return mQuadTriangleVertexBuffer; }
    const IndexStream& getQuadTriangleIndexStream(s32 index = 0) const
    {
        return index == 0 ? mQuadTriangleTriangleIndexStream : mQuadTriangleLineIndexStream;
    }
    const VertexBuffer& getCubeVertexBuffer() const { return mCubeVertexBuffer; }
    const IndexStream& getCubeIndexStream(s32 index = 0) const
    {
        return index == 0 ? mCubeTriangleIndexStream : mCubeLineIndexStream;
    }
    const VertexBuffer& getPyramidVertexBuffer() const { return mPyramidVertexBuffer; }
    const VertexBuffer& getCircleVertexBuffer() const { return mCircleVertexBuffer; }
    const IndexStream& getCircleIndexStream(s32 quality, s32 drawType = cDrawType_Triangle) const
    {
        return mCircleIndexStreams[quality][drawType];
    }
    const VertexBuffer& getSphereVertexBuffer() const { return mSphereVertexBuffer; }
    const VertexBuffer& getHemisphereVertexBuffer() const { return mHemisphereVertexBuffer; }
    const VertexBuffer& getCylinderVertexBuffer() const { return mCylinderVertexBuffer; }
    const VertexBuffer& getCapsuleVertexBuffer() const { return mCapsuleVertexBuffer; }
    const VertexBuffer& getConeVertexBuffer() const { return mConeVertexBuffer; }
    const VertexBuffer& getTorusVertexBuffer() const { return mTorusVertexBuffer; }
    const IndexStream& getSphereIndexStream(s32 quality, s32 drawType = cDrawType_Triangle) const
    {
        return u32(quality) < cQuality_Num ? mSphereIndexStreams[quality][drawType] :
                                             mSphereIndexStreams[0][drawType];
    }
    const IndexStream& getConeTriangleIndexStream(s32 quality) const
    {
        return mConeTriangleIndexStreams[quality];
    }

    const IndexStream& getCylinderTriangleIndexStream(s32 quality) const
    {
        return mCylinderTriangleIndexStreams[quality];
    }

    static u32 calcVtxArrayNumCircle(u32 divNum);
    static u32 calcIdxArrayNumCircle(u32 divNum, DrawType drawType);
    static void setupVtxBufferCircle(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                     u32 divNum);
    static void setupIdxStreamCircle(IndexStream* pIndexStream, GPUMemAddr<u16> addr, u32 divNum,
                                     DrawType drawType);
    static void setupIdxStreamCircle(IndexStream* pIndexStream, GPUMemAddr<u32> addr, u32 divNum,
                                     DrawType drawType);

    static u32 calcVtxArrayNumSphere(u32 divNumU, u32 divNumV);
    static u32 calcIdxArrayNumSphere(u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupVtxBufferSphere(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                     u32 divNumU, u32 divNumV);
    static void setupIdxStreamSphere(IndexStream* pIndexStream, GPUMemAddr<u16> addr, u32 divNumU,
                                     u32 divNumV, DrawType drawType);
    static void setupIdxStreamSphere(IndexStream* pIndexStream, GPUMemAddr<u32> addr, u32 divNumU,
                                     u32 divNumV, DrawType drawType);

    static u32 calcVtxArrayNumHemisphere(u32 divNumU, u32 divNumV);
    static u32 calcIdxArrayNumHemisphere(u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupVtxBufferHemisphere(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                         u32 divNumU, u32 divNumV);
    static void setupIdxStreamHemisphere(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                         u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupIdxStreamHemisphere(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                         u32 divNumU, u32 divNumV, DrawType drawType);

    static u32 calcVtxArrayNumCylinder(u32 divNumU, u32 divNumV);
    static u32 calcIdxArrayNumCylinder(u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupVtxBufferCylinder(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                       u32 divNumU, u32 divNumV);
    static void setupIdxStreamCylinder(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                       u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupIdxStreamCylinder(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                       u32 divNumU, u32 divNumV, DrawType drawType);

    static u32 calcVtxArrayNumCapsule(u32 divNumU, u32 divNumV, u32 divNumH);
    static u32 calcIdxArrayNumCapsule(u32 divNumU, u32 divNumV, u32 divNumH, DrawType drawType);
    static void setupVtxBufferCapsule(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                      u32 divNumU, u32 divNumV, u32 divNumH);
    static void setupIdxStreamCapsule(IndexStream* pIndexStream, GPUMemAddr<u16> addr, u32 divNumU,
                                      u32 divNumV, u32 divNumH, DrawType drawType);
    static void setupIdxStreamCapsule(IndexStream* pIndexStream, GPUMemAddr<u32> addr, u32 divNumU,
                                      u32 divNumV, u32 divNumH, DrawType drawType);

    static u32 calcVtxArrayNumCone(u32 divNumU, u32 divNumV);
    static u32 calcIdxArrayNumCone(u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupVtxBufferCone(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                   u32 divNumU, u32 divNumV);
    static void setupIdxStreamCone(IndexStream* pIndexStream, GPUMemAddr<u16> addr, u32 divNumU,
                                   u32 divNumV, DrawType drawType);
    static void setupIdxStreamCone(IndexStream* pIndexStream, GPUMemAddr<u32> addr, u32 divNumU,
                                   u32 divNumV, DrawType drawType);

    static u32 calcVtxArrayNumTorus(u32 divNumU, u32 divNumV);
    static u32 calcIdxArrayNumTorus(u32 divNumU, u32 divNumV, DrawType drawType);
    static void setupVtxBufferTorus(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                    u32 divNumU, u32 divNumV, f32 radius, f32 tubeRadius,
                                    s32 startIndex, s32 step);
    static void setupIdxStreamTorus(IndexStream* pIndexStream, GPUMemAddr<u16> addr, u32 divNumU,
                                    u32 divNumV, DrawType drawType);
    static void setupIdxStreamTorus(IndexStream* pIndexStream, GPUMemAddr<u32> addr, u32 divNumU,
                                    u32 divNumV, DrawType drawType);

    static u32 calcVtxArrayNumGridQuad(u32 divNumX, u32 divNumY);
    static u32 calcIdxArrayNumGridQuad(u32 divNumX, u32 divNumY);
    static void setupIdxStreamGridQuad(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                       u32 divNumX, u32 divNumY);
    static void setupIdxStreamGridQuad(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                       u32 divNumX, u32 divNumY);
    static void setupVtxBufferGridQuad(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                       u32 divNumX, u32 divNumY);

    static u32 calcVtxArrayNumGridCube(u32 divNumX, u32 divNumY, u32 divNumZ);
    static u32 calcIdxArrayNumGridCube(u32 divNumX, u32 divNumY, u32 divNumZ);
    static void setupIdxStreamGridCube(IndexStream* pIndexStream, GPUMemAddr<u16> addr,
                                       u32 divNumX, u32 divNumY, u32 divNumZ);
    static void setupIdxStreamGridCube(IndexStream* pIndexStream, GPUMemAddr<u32> addr,
                                       u32 divNumX, u32 divNumY, u32 divNumZ);
    static void setupVtxBufferGridCube(VertexBuffer* pVertexBuffer, GPUMemAddr<Vertex> addr,
                                       u32 divNumX, u32 divNumY, u32 divNumZ);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    void setUpStreamQuad_(sead::Heap* pHeap);
    void setUpStreamQuadTriangle_(sead::Heap* pHeap);
    void setUpStreamCube_(sead::Heap* pHeap);
    void setUpStreamPyramid_(sead::Heap* pHeap);
    void setUpStreamCircle_(u32 divNum, sead::Heap* pHeap);
    void setUpStreamSphere_(u32 divNumU, u32 divNumV, sead::Heap* pHeap);
    void setUpStreamHemisphere_(u32 divNumU, u32 divNumV, sead::Heap* pHeap);
    void setUpStreamCylinder_(u32 divNumU, u32 divNumV, sead::Heap* pHeap);
    void setUpStreamCapsule_(u32 divNumU, u32 divNumV, u32 divNumH, sead::Heap* pHeap);
    void setUpStreamCone_(u32 divNumU, u32 divNumV, sead::Heap* pHeap);
    void setUpStreamTorus_(u32 divNumU, u32 divNumV, sead::Heap* pHeap, f32 radius,
                           f32 tubeRadius, s32 startIndex, s32 step);

    static void setUpStreams_(VertexBuffer* pVertexBuffer);

    static u32 calcIdxArrayNumCircle_(u32 divNum, DrawType drawType, Quality quality);
    template <typename T>
    static void setupIdxStreamCircle_(IndexStream* pIndexStream, GPUMemAddr<T> addr, u32 divNum,
                                      DrawType drawType, Quality quality);
    static u32 calcIdxArrayNumSphere_(u32 divNumU, u32 divNumV, DrawType drawType,
                                      Quality quality);
    template <typename T>
    static void setupIdxStreamSphere_(IndexStream* pIndexStream, GPUMemAddr<T> addr, u32 divNumU,
                                      u32 divNumV, DrawType drawType, Quality quality);
    static u32 calcIdxArrayNumHemisphere_(u32 divNumU, u32 divNumV, DrawType drawType,
                                          Quality quality);
    template <typename T>
    static void setupIdxStreamHemisphere_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                          u32 divNumU, u32 divNumV, DrawType drawType,
                                          Quality quality);
    static u32 calcIdxArrayNumCylinder_(u32 divNumU, u32 divNumV, DrawType drawType,
                                        Quality quality);
    template <typename T>
    static void setupIdxStreamCylinder_(IndexStream* pIndexStream, GPUMemAddr<T> addr,
                                        u32 divNumU, u32 divNumV, DrawType drawType,
                                        Quality quality);
    static u32 calcIdxArrayNumCapsule_(u32 divNumU, u32 divNumV, u32 divNumH, DrawType drawType,
                                       Quality quality);
    template <typename T>
    static void setupIdxStreamCapsule_(IndexStream* pIndexStream, GPUMemAddr<T> addr, u32 divNumU,
                                       u32 divNumV, u32 divNumH, DrawType drawType,
                                       Quality quality);
    static u32 calcIdxArrayNumCone_(u32 divNumU, u32 divNumV, DrawType drawType, Quality quality);
    template <typename T>
    static void setupIdxStreamCone_(IndexStream* pIndexStream, GPUMemAddr<T> addr, u32 divNumU,
                                    u32 divNumV, DrawType drawType, Quality quality);
    static u32 calcIdxArrayNumTorus_(u32 divNumU, u32 divNumV, DrawType drawType,
                                     Quality quality);
    template <typename T>
    static void setupIdxStreamTorus_(IndexStream* pIndexStream, GPUMemAddr<T> addr, u32 divNumU,
                                     u32 divNumV, DrawType drawType, Quality quality);
    static sead::Vector3f calcTorusCircleCenter_(s32 windP, s32 windQ, f32 t, f32 radius);

    GPUMemBlock<Vertex> mQuadVertexBlock;
    GPUMemBlock<u16> mQuadTriangleIndexBlock;
    GPUMemBlock<u16> mQuadLineIndexBlock;
    VertexBuffer mQuadVertexBuffer;
    IndexStream mQuadTriangleIndexStream;
    IndexStream mQuadLineIndexStream;
    GPUMemBlock<Vertex> mQuadTriangleVertexBlock;
    GPUMemBlock<u16> mQuadTriangleTriangleIndexBlock;
    GPUMemBlock<u16> mQuadTriangleLineIndexBlock;
    VertexBuffer mQuadTriangleVertexBuffer;
    IndexStream mQuadTriangleTriangleIndexStream;
    IndexStream mQuadTriangleLineIndexStream;
    GPUMemBlock<Vertex> mCubeVertexBlock;
    GPUMemBlock<u16> mCubeTriangleIndexBlock;
    GPUMemBlock<u16> mCubeLineIndexBlock;
    VertexBuffer mCubeVertexBuffer;
    IndexStream mCubeTriangleIndexStream;
    IndexStream mCubeLineIndexStream;
    GPUMemBlock<Vertex> mPyramidVertexBlock;
    GPUMemBlock<u16> mPyramidTriangleIndexBlock;
    GPUMemBlock<u16> mPyramidLineIndexBlock;
    VertexBuffer mPyramidVertexBuffer;
    IndexStream mPyramidTriangleIndexStream;
    IndexStream mPyramidLineIndexStream;
    GPUMemBlock<Vertex> mCircleVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cDrawType_Num> mCircleIndexBlocks[cQuality_Num];
    VertexBuffer mCircleVertexBuffer;
    sead::SafeArray<IndexStream, cDrawType_Num> mCircleIndexStreams[cQuality_Num];
    GPUMemBlock<Vertex> mSphereVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cDrawType_Num> mSphereIndexBlocks[cQuality_Num];
    VertexBuffer mSphereVertexBuffer;
    sead::SafeArray<IndexStream, cDrawType_Num> mSphereIndexStreams[cQuality_Num];
    GPUMemBlock<Vertex> mHemisphereVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cDrawType_Num> mHemisphereIndexBlocks[cQuality_Num];
    VertexBuffer mHemisphereVertexBuffer;
    sead::SafeArray<IndexStream, cDrawType_Num> mHemisphereIndexStreams[cQuality_Num];
    GPUMemBlock<Vertex> mCylinderVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mCylinderTriangleIndexBlocks;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mCylinderLineIndexBlocks;
    VertexBuffer mCylinderVertexBuffer;
    sead::SafeArray<IndexStream, cQuality_Num> mCylinderTriangleIndexStreams;
    sead::SafeArray<IndexStream, cQuality_Num> mCylinderLineIndexStreams;
    GPUMemBlock<Vertex> mCapsuleVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mCapsuleTriangleIndexBlocks;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mCapsuleLineIndexBlocks;
    VertexBuffer mCapsuleVertexBuffer;
    sead::SafeArray<IndexStream, cQuality_Num> mCapsuleTriangleIndexStreams;
    sead::SafeArray<IndexStream, cQuality_Num> mCapsuleLineIndexStreams;
    GPUMemBlock<Vertex> mConeVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mConeTriangleIndexBlocks;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mConeLineIndexBlocks;
    VertexBuffer mConeVertexBuffer;
    sead::SafeArray<IndexStream, cQuality_Num> mConeTriangleIndexStreams;
    sead::SafeArray<IndexStream, cQuality_Num> mConeLineIndexStreams;
    GPUMemBlock<Vertex> mTorusVertexBlock;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mTorusTriangleIndexBlocks;
    sead::SafeArray<GPUMemBlock<u16>, cQuality_Num> mTorusLineIndexBlocks;
    VertexBuffer mTorusVertexBuffer;
    sead::SafeArray<IndexStream, cQuality_Num> mTorusTriangleIndexStreams;
    sead::SafeArray<IndexStream, cQuality_Num> mTorusLineIndexStreams;
};
static_assert(sizeof(PrimitiveShape) == 0x31f8);

}  // namespace agl::utl
