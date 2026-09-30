#include <nn/ui2d/ui2d_TextureContainer.h>
#include <nn/ui2d/ui2d_RenderTargetTextureInfo.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/util/util_StringUtil.h>
#include <new>

namespace nn::ui2d {
// texture is the wrapper held by this link; owned also transfers its GPU resources.
TextureRefLink::TextureRefLink(TextureInfo* texture, bool owned) : mTexture(texture), mOwned(owned) { mName[0] = 0; }
TextureRefLink::~TextureRefLink() = default;
// device releases owned GPU resources. The wrapper itself is always destroyed.
void TextureRefLink::Finalize(nn::gfx::Device* device) {
    if (mOwned) mTexture->Finalize(device);
    TextureInfo* texture = mTexture;

    if (texture) { texture->~TextureInfo(); Layout::FreeMemory(texture); }
}

// name is copied into the fixed-length registration name.
void TextureRefLink::SetName(const char* name) { nn::util::Strlcpy(mName, name, sizeof(mName)); }
TextureContainer::~TextureContainer() = default;
// device finalizes the owned resources before all wrappers and links are freed.
void TextureContainer::Finalize(nn::gfx::Device* device) {
    while (!mTextures.empty()) {
        auto it = mTextures.begin();
        TextureRefLink* link = &*it;
        link->Finalize(device);
        mTextures.erase(it);
        link->~TextureRefLink();
        Layout::FreeMemory(link);
    }
}

// name identifies the new resource texture, which owns its GPU resources.
ResourceTextureInfo* TextureContainer::RegisterResourceTexture(const char* name) {
    void* memory = Layout::AllocateMemory(sizeof(ResourceTextureInfo));
    auto* texture = memory ? new (memory) ResourceTextureInfo : nullptr;
    memory = Layout::AllocateMemory(sizeof(TextureRefLink));

    if (!memory) return nullptr;
    auto* link = new (memory) TextureRefLink(texture, true);
    link->SetName(name);
    mTextures.push_back(*link);
    return texture;
}

// name identifies the wrapper; owned controls GPU-resource finalization.
PlacementTextureInfo* TextureContainer::RegisterPlacementTexture(const char* name, bool owned) {
    void* memory = Layout::AllocateMemory(sizeof(PlacementTextureInfo));
    auto* texture = memory ? new (memory) PlacementTextureInfo : nullptr;
    memory = Layout::AllocateMemory(sizeof(TextureRefLink));

    if (!memory) return nullptr;
    auto* link = new (memory) TextureRefLink(texture, owned);
    link->SetName(name);
    mTextures.push_back(*link);
    return texture;
}

// name identifies the render target; owned controls GPU-resource finalization.
RenderTargetTextureInfo* TextureContainer::RegisterRenderTargetTexture(const char* name, bool owned) {
    void* memory = Layout::AllocateMemory(sizeof(RenderTargetTextureInfo));
    auto* texture = memory ? new (memory) RenderTargetTextureInfo : nullptr;
    memory = Layout::AllocateMemory(sizeof(TextureRefLink));

    if (!memory) return nullptr;
    auto* link = new (memory) TextureRefLink(texture, owned);
    link->SetName(name);
    mTextures.push_back(*link);
    return texture;
}

// texture identifies the registration to remove, retaining the texture wrapper.
void TextureContainer::UnregisterTexture(TextureInfo* texture) {
    for (auto it = mTextures.begin(); it != mTextures.end(); ++it) {
        if (it->mTexture == texture) {
            auto* link = &*it;
            mTextures.erase(it);
            Layout::FreeMemory(link);
            return;
        }
    }
}

// callback allocates each missing owned descriptor; argument is its context.
void TextureContainer::RegisterTextureViewToDescriptorPool(RegisterCallback callback, void* argument) {
    for (auto& link : mTextures) {
        if (!link.mOwned) continue;
        const auto descriptor = link.mTexture->mDescriptor;

        if (!descriptor.IsValid())
            callback(&link.mTexture->mDescriptor, *link.mTexture->GetTextureView(), argument);
    }
}

// callback releases each valid owned descriptor; argument is its context.
void TextureContainer::UnregisterTextureViewFromDescriptorPool(UnregisterCallback callback, void* argument) {
    for (auto& link : mTextures) {
        if (!link.mOwned) continue;
        const auto descriptor = link.mTexture->mDescriptor;

        if (descriptor.IsValid()) {
            callback(&link.mTexture->mDescriptor, *link.mTexture->GetTextureView(), argument);
            link.mTexture->mDescriptor.Invalidate();
        }
    }
}

// name is compared up to the registration's 128-byte name limit.
TextureInfo* TextureContainer::FindTextureByName(const char* name) const {
    for (auto& link : mTextures) {
        bool same = true;

        for (size_t i = 0; i < sizeof(link.mName); ++i) {
            if (name[i] != link.mName[i]) { same = false; break; }
            if (!name[i]) break;
        }

        if (same) return link.mTexture;
    }

    return nullptr;
}

// device is unused: a placement wrapper does not own a GPU texture.
void PlacementTextureInfo::Finalize(nn::gfx::Device* device) {}
bool PlacementTextureInfo::IsValid() const { return mDescriptor.IsValid(); }
const nn::gfx::TextureView* PlacementTextureInfo::GetTextureView() const { return nullptr; }
nn::gfx::TextureView* PlacementTextureInfo::GetTextureView() { return nullptr; }
}
