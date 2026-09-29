#pragma once

#include "devenv/seadFontMgr.h"

namespace sead
{
class DebugFontMgrJis1Nvn : public FontBase
{
    SEAD_SINGLETON_DISPOSER(DebugFontMgrJis1Nvn)
public:
    DebugFontMgrJis1Nvn();
    ~DebugFontMgrJis1Nvn() override;

    float getHeight() const override;
    float getWidth() const override;
    float getCharWidth(char16_t c) const override;
    u32 getEncoding() const override { return 2; }
    u32 getMaxDrawNum() const override;
    void begin(DrawContext* pDrawContext) const override;
    void end(DrawContext* pDrawContext) const override;
    void print(DrawContext* pDrawContext, const Projection& rProjection, const Camera& rCamera,
               const Matrix34f& rMatrix, const Color4f& rColor, const void* pText,
               int length) const override;

    static void setTextureID(u32 id);

    void initialize(Heap* pHeap, const char* pShaderPath, const char* pFontPath,
                    const char* pTablePath, u32 uniformBufferSize);
    void initializeFromBinary(Heap* pHeap, void* pShaderBinary, u64 shaderSize, void* pFontBinary,
                              u64 fontSize, const void* pTableBinary, u32 uniformBufferSize);
    void swapUniformBlockBuffer();
    u32 searchCharIndexFormCharCode_(u32 code) const;

private:
    static u32 sTextureID;

    NVNprogram mNvnProgram;
    NVNtexture mNvnTexture;
    NVNtextureHandle mNvnTextureHandle;
    NVNmemoryPool mFontMemoryPool;
    NVNmemoryPool mShaderMemoryPool;
    NVNmemoryPool mUniformMemoryPool;
    NVNbuffer mShaderBuffer;
    u32 mUniformBufferSize = 0;
    const void* mCharCodeTable = nullptr;
    NVNbuffer mUniformBuffer;
    void* mUniformBufferMap = nullptr;
    mutable UniformBlockBuffer mUniformBlockBuffer;
    mutable bool mIsUniformBufferFull = false;
    u8 mFontSizeType = 0;
};
static_assert(sizeof(DebugFontMgrJis1Nvn) == 0x538);

class DebugFontMgrNvn : public FontBase
{
    SEAD_SINGLETON_DISPOSER(DebugFontMgrNvn)
public:
    DebugFontMgrNvn();
    ~DebugFontMgrNvn() override;

    float getHeight() const override;
    float getWidth() const override;
    float getCharWidth(char16_t c) const override;
    u32 getEncoding() const override { return 2; }
    u32 getMaxDrawNum() const override;
    void begin(DrawContext* pDrawContext) const override;
    void end(DrawContext* pDrawContext) const override;
    void print(DrawContext* pDrawContext, const Projection& rProjection, const Camera& rCamera,
               const Matrix34f& rMatrix, const Color4f& rColor, const void* pText,
               int length) const override;

    static void setTextureID(u32 id);

    void initialize(Heap* pHeap, const char* pShaderPath, const char* pFontPath,
                    u32 uniformBufferSize);
    void initializeFromBinary(Heap* pHeap, void* pShaderBinary, u64 shaderSize, void* pFontBinary,
                              u64 fontSize, u32 uniformBufferSize);
    void swapUniformBlockBuffer();

private:
    static u32 sTextureID;

    NVNprogram mNvnProgram;
    NVNtexture mNvnTexture;
    NVNtextureHandle mNvnTextureHandle;
    NVNmemoryPool mFontMemoryPool;
    NVNmemoryPool mShaderMemoryPool;
    NVNmemoryPool mUniformMemoryPool;
    NVNbuffer mShaderBuffer;
    u32 mUniformBufferSize = 0;
    u32 _4e4;
    NVNbuffer mUniformBuffer;
    void* mUniformBufferMap = nullptr;
    mutable UniformBlockBuffer mUniformBlockBuffer;
    mutable bool mIsUniformBufferFull = false;
};
static_assert(sizeof(DebugFontMgrNvn) == 0x530);
}  // namespace sead
