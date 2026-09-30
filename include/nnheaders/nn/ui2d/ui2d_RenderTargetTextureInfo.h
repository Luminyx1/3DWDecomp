#pragma once

#include <nn/ui2d/ui2d_TextureInfo.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/gfx/gfx_State.h>

namespace nn::ui2d {
class Layout;
enum RenderTargetTextureLifetime : int;

class RenderTargetTextureInfo : public PlacementTextureInfo {
public:
    NN_RUNTIME_TYPEINFO(PlacementTextureInfo);
    ~RenderTargetTextureInfo() override;
    void Initialize(nn::gfx::Device*, const Layout*, const nn::gfx::TextureInfo&,
                    RenderTargetTextureLifetime);
    void Finalize(nn::gfx::Device*) override;
    int GetFormat() const override { return mFormat; }
    bool IsValid() const override { return mFormat != 0; }
    const nn::gfx::TextureView* GetTextureView() const override { return mTextureView; }
    nn::gfx::TextureView* GetTextureView() override { return mTextureView; }
    void* GetPrivateTextureInstancePtr() const override { return mPrivateTextureInstance; }

    nn::gfx::Texture* mTexture;
    nn::gfx::TextureView* mTextureView;
    nn::gfx::DescriptorSlot* mTextureSlot;
    const Layout* mLayout;
    void* mPrivateTextureInstance;
    nn::gfx::ColorTargetView mColorTarget;
    nn::gfx::ViewportScissorState mViewportScissorState;
    nn::gfx::ImageFormat mFormat;
    RenderTargetTextureLifetime mLifetime;
};

class DummyRenderTargetTextureInfo : public TextureInfo {
public:
    NN_RUNTIME_TYPEINFO(TextureInfo);
    explicit DummyRenderTargetTextureInfo(const char* name);
    ~DummyRenderTargetTextureInfo() override;
    void Finalize(nn::gfx::Device*) override;
    TextureSize GetSize() const override { return TextureSize(0, 0); }
    bool IsValid() const override { return true; }
    const nn::gfx::TextureView* GetTextureView() const override { return nullptr; }
    nn::gfx::TextureView* GetTextureView() override { return nullptr; }
    char* mName;
};
}
