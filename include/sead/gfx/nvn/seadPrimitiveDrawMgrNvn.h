#pragma once

#include <devenv/seadFontMgr.h>
#include <gfx/seadColor.h>
#include <heap/seadDisposer.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <nvn/nvn.h>
#include <prim/seadSafeString.h>

namespace sead
{
class DrawContext;
class Heap;
class Texture;

class PrimitiveDrawMgrBase
{
public:
    virtual void prepareFromBinaryImpl(Heap* pHeap, const void* pBinary, u32 binarySize) = 0;
    virtual void prepareImpl(Heap* pHeap, const SafeString& rPath) = 0;
    virtual void beginImpl(DrawContext* pDrawContext, const Matrix34f& rViewMatrix,
                           const Matrix44f& rProjectionMatrix) = 0;
    virtual void endImpl(DrawContext* pDrawContext) = 0;
    virtual void drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                              const Color4f& rColor0, const Color4f& rColor1) = 0;
    virtual void drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                              const Texture& rTexture, const Color4f& rColor0,
                              const Color4f& rColor1, const Vector2f& rUVSrc,
                              const Vector2f& rUVSize) = 0;
    virtual void drawBoxImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                             const Color4f& rColor0, const Color4f& rColor1) = 0;
    virtual void drawCubeImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                              const Color4f& rColor0, const Color4f& rColor1) = 0;
    virtual void drawWireCubeImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                  const Color4f& rColor0, const Color4f& rColor1) = 0;
    virtual void drawLineImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                              const Color4f& rColor0, const Color4f& rColor1) = 0;
    virtual void drawSphere4x8Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                   const Color4f& rNorth, const Color4f& rSouth) = 0;
    virtual void drawSphere8x16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                    const Color4f& rNorth, const Color4f& rSouth) = 0;
    virtual void drawDisk16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                const Color4f& rCenter, const Color4f& rEdge) = 0;
    virtual void drawDisk32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                const Color4f& rCenter, const Color4f& rEdge) = 0;
    virtual void drawCircle16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                  const Color4f& rEdge) = 0;
    virtual void drawCircle32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                  const Color4f& rEdge) = 0;
    virtual void drawCylinder16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                    const Color4f& rTop, const Color4f& rBottom) = 0;
    virtual void drawCylinder32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                                    const Color4f& rTop, const Color4f& rBottom) = 0;
    virtual ~PrimitiveDrawMgrBase() = default;
};

class PrimitiveDrawMgrNvn : public PrimitiveDrawMgrBase
{
    SEAD_SINGLETON_DISPOSER(PrimitiveDrawMgrNvn)
public:
    PrimitiveDrawMgrNvn();
    ~PrimitiveDrawMgrNvn() override;

    void prepareFromBinaryImpl(Heap* pHeap, const void* pBinary, u32 binarySize) override;
    void prepareImpl(Heap* pHeap, const SafeString& rPath) override;
    void beginImpl(DrawContext* pDrawContext, const Matrix34f& rViewMatrix,
                   const Matrix44f& rProjectionMatrix) override;
    void endImpl(DrawContext* pDrawContext) override;
    void drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                      const Color4f& rColor0, const Color4f& rColor1) override;
    void drawQuadImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                      const Texture& rTexture, const Color4f& rColor0, const Color4f& rColor1,
                      const Vector2f& rUVSrc, const Vector2f& rUVSize) override;
    void drawBoxImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                     const Color4f& rColor0, const Color4f& rColor1) override;
    void drawCubeImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                      const Color4f& rColor0, const Color4f& rColor1) override;
    void drawWireCubeImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                          const Color4f& rColor0, const Color4f& rColor1) override;
    void drawLineImpl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                      const Color4f& rColor0, const Color4f& rColor1) override;
    void drawSphere4x8Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                           const Color4f& rNorth, const Color4f& rSouth) override;
    void drawSphere8x16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                            const Color4f& rNorth, const Color4f& rSouth) override;
    void drawDisk16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                        const Color4f& rCenter, const Color4f& rEdge) override;
    void drawDisk32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                        const Color4f& rCenter, const Color4f& rEdge) override;
    void drawCircle16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                          const Color4f& rEdge) override;
    void drawCircle32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                          const Color4f& rEdge) override;
    void drawCylinder16Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                            const Color4f& rTop, const Color4f& rBottom) override;
    void drawCylinder32Impl(DrawContext* pDrawContext, const Matrix34f& rModelMatrix,
                            const Color4f& rTop, const Color4f& rBottom) override;

    void swapUniformBlockBuffer();

    void setUniformBufferSize(u32 size) { mUniformBufferSize = size; }
    void setTextureEnable(bool enable) { mIsTextureEnable = enable; }

private:
    void setupNVNBuffer_(NVNbuffer* pBuffer, NVNmemoryPool* pMemoryPool, size_t* pOffset,
                         size_t size);
    void drawImpl_(NVNcommandBuffer* pCommandBuffer, NVNdrawPrimitive primitive,
                   const Matrix34f& rModelMatrix, const Color4f& rColor0, const Color4f& rColor1,
                   NVNbuffer* pVertexBuffer, u32 vertexNum, NVNbuffer* pIndexBuffer, u32 indexNum,
                   const Texture* pTexture, const Vector2f* pUVSrc, const Vector2f* pUVSize);

    NVNprogram mNvnProgram;
    NVNbuffer mShaderBuffer;
    NVNmemoryPool mShaderMemoryPool;
    NVNvertexAttribState mVertexAttribStates[3];
    u32 _224;
    NVNvertexStreamState mVertexStreamState;
    NVNmemoryPool mMemoryPool;
    NVNbuffer mQuadVertexBuffer;
    NVNbuffer mQuadIndexBuffer;
    NVNbuffer mBoxIndexBuffer;
    NVNbuffer mLineVertexBuffer;
    NVNbuffer mLineIndexBuffer;
    NVNbuffer mCubeVertexBuffer;
    NVNbuffer mCubeIndexBuffer;
    NVNbuffer mWireCubeVertexBuffer;
    NVNbuffer mWireCubeIndexBuffer;
    NVNbuffer mSphereSVertexBuffer;
    NVNbuffer mSphereSIndexBuffer;
    NVNbuffer mSphereLVertexBuffer;
    NVNbuffer mSphereLIndexBuffer;
    NVNbuffer mDiskSVertexBuffer;
    NVNbuffer mDiskSIndexBuffer;
    NVNbuffer mDiskLVertexBuffer;
    NVNbuffer mDiskLIndexBuffer;
    NVNbuffer mCircleSIndexBuffer;
    NVNbuffer mCircleLIndexBuffer;
    NVNbuffer mCylinderSVertexBuffer;
    NVNbuffer mCylinderSIndexBuffer;
    NVNbuffer mCylinderLVertexBuffer;
    NVNbuffer mCylinderLIndexBuffer;
    NVNbuffer mUniformBuffer;
    void* mUniformBufferMap = nullptr;
    UniformBlockBuffer mUniformBlockBuffer;
    u32 mUniformBufferSize = 0x32000;
    bool mIsUniformBufferFull = false;
    bool mIsTextureEnable = false;
};
static_assert(sizeof(PrimitiveDrawMgrNvn) == 0x7c8);

}  // namespace sead
