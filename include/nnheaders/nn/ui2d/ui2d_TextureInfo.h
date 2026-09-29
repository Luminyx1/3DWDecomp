#pragma once

#include <nn/font/font_Util.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_Types.h>

namespace nn::ui2d {

struct TextureSize {
    u16 width;
    u16 height;
    TextureSize(const TextureSize&);
};

class TextureInfo {
public:
    NN_RUNTIME_TYPEINFO_BASE();
    virtual ~TextureInfo() = default;
    virtual void Finalize(nn::gfx::Device*) = 0;
    virtual TextureSize GetSize() const = 0;
    virtual int GetFormat() const { return 0; }
    virtual bool IsValid() const = 0;
    virtual const nn::gfx::TextureView* GetTextureView() const = 0;
    virtual nn::gfx::TextureView* GetTextureView() = 0;
    virtual void* GetPrivateTextureInstancePtr() const { return nullptr; }

    nn::gfx::DescriptorSlot mDescriptor;
};

class PlacementTextureInfo : public TextureInfo {
public:
    PlacementTextureInfo() : mWidth(0), mHeight(0) {}
    const nn::font::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const override;
    void Finalize(nn::gfx::Device*) override;
    TextureSize GetSize() const override;
    bool IsValid() const override;
    const nn::gfx::TextureView* GetTextureView() const override;
    nn::gfx::TextureView* GetTextureView() override;

    u16 mWidth;
    u16 mHeight;
};

}  // namespace nn::ui2d
