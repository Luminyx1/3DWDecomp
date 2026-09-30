#include <nn/ui2d/ui2d_MultiArcResourceAccessorEx.h>
#include <nn/ui2d/ui2d_FontMgr.h>
namespace nn::ui2d {
// archives supplies archive metadata; fonts provides the shared font registrations.
MultiArcResourceAccessorEx::MultiArcResourceAccessorEx(const ArcResourceMgr* archives, const FontMgr* fonts) : mArchiveManager(archives), mFontManager(fonts) {}
// archive identifies an attached archive by the start of its binary data.
bool MultiArcResourceAccessorEx::IsArchiveAttached(void* archive) {
    for (auto& link : mArchives) if (link.extractor.m_pArchiveBlockHeader == archive) return true;
    return false;
}

// name selects a shared font; device is unused because the manager owns its resources.
nn::font::Font* MultiArcResourceAccessorEx::AcquireFont(nn::gfx::Device* device, const char* name) {
    return const_cast<nn::font::Font*>(mFontManager->GetFont(name));
}

// type, callback, and argument are unused: this accessor does not enumerate resources.
void MultiArcResourceAccessorEx::FindResourceByType(u32 type, ResourceCallback callback, void* argument) const {}
// callback allocates missing texture descriptors; argument is its context.
void MultiArcResourceAccessorEx::RegisterTextureViewToDescriptorPool(RegisterTextureView callback, void* argument) {
    for (auto& link : mTextures) {
        TextureInfo& texture = link.texture;
        if (!texture.mDescriptor.IsValid()) callback(&texture.mDescriptor, *texture.GetTextureView(), argument);
    }
}

// callback releases texture descriptors; argument is its context.
void MultiArcResourceAccessorEx::UnregisterTextureViewFromDescriptorPool(UnregisterTextureView callback, void* argument) {
    for (auto& link : mTextures) {
        TextureInfo& texture = link.texture;
        callback(&texture.mDescriptor, *texture.GetTextureView(), argument);
        texture.mDescriptor.Invalidate();
    }
}

// device releases cached texture and shader GPU resources.
void MultiArcResourceAccessorEx::Finalize(nn::gfx::Device* device) {
    for (auto& link : mTextures) static_cast<TextureInfo&>(link.texture).Finalize(device);
    mShaders.Finalize(device);
    ResourceAccessor::Finalize(device);
}
}
