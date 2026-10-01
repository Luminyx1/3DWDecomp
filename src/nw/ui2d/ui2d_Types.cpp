#include <nn/ui2d/ui2d_RenderTargetTextureInfo.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/gfx/gfx_TextureInfo.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_StateInfo.h>

namespace nn::ui2d {
namespace detail { char* AllocateAndCopyString(const char* source); }
RenderTargetTextureInfo::~RenderTargetTextureInfo() = default;
TextureSize PlacementTextureInfo::GetSize() const {
    TextureSize size(0, 0);
    size.width = mWidth;
    size.height = mHeight;
    return size;
}

// device owns the GPU views; layout identifies their owner. info supplies texture
// dimensions and format; lifetime requests the allocation's retention period.
void RenderTargetTextureInfo::Initialize(nn::gfx::Device* device, const Layout* layout,
                                        const nn::gfx::TextureInfo& info,
                                        RenderTargetTextureLifetime lifetime) {
    mLayout = layout;
    mLifetime = Layout::g_pCreateRenderTargetTextureResourceCallback(
        &mTexture, &mTextureView, &mTextureSlot, layout, info,
        Layout::g_pRenderTargetTextureCallbackUserData, lifetime);
    mDescriptor = *mTextureSlot;
    {
        nn::gfx::ColorTargetViewInfo target;
        target.SetDefault();
        target.SetImageDimension(info.GetMultisampleCount() > 1 ? nn::gfx::ImageDimension_2dMultisample : nn::gfx::ImageDimension_2d);
        target.EditArrayRange().SetBaseArrayIndex(0);
        target.EditArrayRange().SetArrayLength(1);
        target.SetImageFormat(info.GetImageFormat());
        target.SetTexturePtr(mTexture);
        mColorTarget.Initialize(device, target);
    }

    nn::gfx::ViewportScissorStateInfo state;
    state.SetDefault();
    state.SetScissorEnabled(true);
    nn::gfx::ViewportStateInfo viewport;
    viewport.SetDefault();
    viewport.SetOriginX(0);
    viewport.SetOriginY(0);
    viewport.SetWidth(info.GetWidth());
    viewport.SetHeight(info.GetHeight());
    nn::gfx::ScissorStateInfo scissor;
    scissor.SetDefault();
    scissor.SetOriginX(0);
    scissor.SetOriginY(0);
    scissor.SetWidth(info.GetWidth());
    scissor.SetHeight(info.GetHeight());
    state.SetViewportStateInfoArray(&viewport, 1);
    state.SetScissorStateInfoArray(&scissor, 1);
    mViewportScissorState.Initialize(device, state);
    TextureSize size(info.GetWidth(), info.GetHeight());
    mWidth = size.width;
    mHeight = size.height;
    mFormat = info.GetImageFormat();
}

// device releases the GPU states before the callback releases the texture allocation.
void RenderTargetTextureInfo::Finalize(nn::gfx::Device* device) {
    if (IsValid()) {
        mViewportScissorState.Finalize(device);
        mColorTarget.Finalize(device);
        Layout::g_pDestroyRenderTargetTextureResourceCallback(mTexture, mTextureView, mTextureSlot,
            mLayout, Layout::g_pRenderTargetTextureCallbackUserData, mLifetime);
        mFormat = nn::gfx::ImageFormat(0);
        mTexture = nullptr;
        mTextureView = nullptr;
        mTextureSlot = nullptr;
    }
}

// name is copied so the placeholder can outlive the caller's name buffer.
DummyRenderTargetTextureInfo::DummyRenderTargetTextureInfo(const char* name) : mName(nullptr) {
    mName = detail::AllocateAndCopyString(name);
}

DummyRenderTargetTextureInfo::~DummyRenderTargetTextureInfo() {}
// device is accepted by the common texture interface; this placeholder owns only a name.
void DummyRenderTargetTextureInfo::Finalize(nn::gfx::Device* device) {
    if (mName != nullptr) { Layout::FreeMemory(mName); mName = nullptr; }
}
}
