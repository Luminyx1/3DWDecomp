#pragma once

#include <nn/ui2d/ui2d_TextureInfo.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::gfx { class ResTexture; }

namespace nn::ui2d {

class ResourceTextureInfo : public TextureInfo {
public:
    ResourceTextureInfo() : m_pResource(nullptr) {}
    NN_RUNTIME_TYPEINFO(TextureInfo);
    ~ResourceTextureInfo() override = default;
    void Finalize(nn::gfx::Device* pDevice) override;
    TextureSize GetSize() const override;
    int GetFormat() const override;
    bool IsValid() const override;
    const nn::gfx::TextureView* GetTextureView() const override;
    nn::gfx::TextureView* GetTextureView() override;

    nn::gfx::ResTexture* m_pResource;
};

static_assert(sizeof(ResourceTextureInfo) == 0x18, "ResourceTextureInfo size");
class RenderTargetTextureInfo;
class TextureRefLink {
public:
    TextureRefLink(TextureInfo* texture, bool owned);
    ~TextureRefLink();
    void Finalize(nn::gfx::Device* device);
    void SetName(const char* name);
    nn::util::IntrusiveListNode m_Link;
    char mName[128];
    TextureInfo* mTexture;
    bool mOwned;
};
static_assert(sizeof(TextureRefLink) == 0xa0, "TextureRefLink size");
class TextureContainer {
public:
    using List = nn::util::IntrusiveList<TextureRefLink, nn::util::IntrusiveListMemberNodeTraits<TextureRefLink, &TextureRefLink::m_Link>>;
    using RegisterCallback = bool (*)(nn::gfx::DescriptorSlot*, const nn::gfx::TextureView&, void*);
    using UnregisterCallback = void (*)(nn::gfx::DescriptorSlot*, const nn::gfx::TextureView&, void*);
    ~TextureContainer();
    void Finalize(nn::gfx::Device* device);
    ResourceTextureInfo* RegisterResourceTexture(const char* name);
    PlacementTextureInfo* RegisterPlacementTexture(const char* name, bool owned);
    RenderTargetTextureInfo* RegisterRenderTargetTexture(const char* name, bool owned);
    void UnregisterTexture(TextureInfo* texture);
    void RegisterTextureViewToDescriptorPool(RegisterCallback callback, void* argument);
    void UnregisterTextureViewFromDescriptorPool(UnregisterCallback callback, void* argument);
    TextureInfo* FindTextureByName(const char* name) const;
    List mTextures;
};
}  // namespace nn::ui2d
