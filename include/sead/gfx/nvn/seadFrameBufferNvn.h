#pragma once

#include <container/seadSafeArray.h>
#include <gfx/seadFrameBuffer.h>
#include <nvn/nvn.h>

namespace sead
{
class Heap;

class FrameBufferNvn : public FrameBuffer
{
    SEAD_RTTI_OVERRIDE(FrameBufferNvn, FrameBuffer)
public:
    FrameBufferNvn(const Vector2f& virtualSize, u32 width, u32 height, NVNtexture* pColorTexture,
                   NVNtexture* pDepthTexture)
        : FrameBuffer(virtualSize, 0.0f, 0.0f, f32(width), f32(height)), mColorTexture(pColorTexture),
          mDepthTexture(pDepthTexture)
    {
    }
    ~FrameBufferNvn() override;

    void copyToDisplayBuffer(DrawContext* pDrawContext,
                             const DisplayBuffer* pDisplayBuffer) const override;
    void clear(DrawContext* pDrawContext, u32 clearFlag, const Color4f& rColor, f32 depth,
               u32 stencil) const override;
    void bindImpl_(DrawContext* pDrawContext) const override;

    static FrameBufferNvn* create(Heap* pHeap, const Vector2f& rVirtualSize, u32 width,
                                  u32 height);

    NVNtexture* getColorTexture() const { return mColorTexture; }
    NVNtexture* getDepthTexture() const { return mDepthTexture; }

private:
    NVNtexture* mColorTexture;
    NVNtexture* mDepthTexture;
};
static_assert(sizeof(FrameBufferNvn) == 0x30);

class DisplayBufferNvn : public DisplayBuffer
{
    SEAD_RTTI_OVERRIDE(DisplayBufferNvn, DisplayBuffer)
public:
    static constexpr s32 cTextureNumMax = 3;

    DisplayBufferNvn();

    void presentTextureAndAcquireNext();
    void waitAcquireDone();
    void setPresentInterval(u8 interval);
    void setTripleBuffer(bool isTripleBuffer);
    void setWindowCrop(s32 x, s32 y, s32 w, s32 h);
    void getWindowCrop(s32* pX, s32* pY, s32* pW, s32* pH) const;
    void applyChangeWindowCrop();

    void setNativeWindow(void* pNativeWindow) { mNativeWindow = pNativeWindow; }
    NVNwindow* getWindow() const { return mWindow; }
    NVNsync* getSync() const { return mSync; }
    s32 getTextureIndex() const { return mTextureIndex; }
    NVNtexture* getTexture(s32 index) const { return mTextures[index]; }
    NVNtexture* getAcquiredTexture() const { return mTextures[mTextureIndex]; }

protected:
    void initializeImpl_(Heap* pHeap) override;

private:
    s32 getTextureNum_() const { return mTextureNum < cTextureNumMax ? mTextureNum : cTextureNumMax; }

    NVNwindow* mWindow;
    NVNsync* mSync;
    s32 mTextureIndex;
    SafeArray<NVNtexture*, cTextureNumMax> mTextures;
    void* mNativeWindow;
    u8 mPresentInterval;
    u8 mTextureNum;
    bool mIsWindowCropChanged;
    s32 mWindowCropX;
    s32 mWindowCropY;
    s32 mWindowCropW;
    s32 mWindowCropH;
};
static_assert(sizeof(DisplayBufferNvn) == 0x60);

}  // namespace sead
