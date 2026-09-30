#pragma once
#include <nn/ui2d/ui2d_ResourceAccessor.h>
#include <nn/util/util_IntrusiveList.h>
namespace eui {
class ArcResourceMgr;
class FontMgr;
class MultiArcResourceAccessor : public nn::ui2d::ResourceAccessor {
public:
    MultiArcResourceAccessor(const ArcResourceMgr* pArchives, const FontMgr* pFonts);
    ~MultiArcResourceAccessor() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::ResourceAccessor);
    void RegisterTextureViewToDescriptorPool(nn::ui2d::RegisterTextureView, void*) override;
    void UnregisterTextureViewFromDescriptorPool(nn::ui2d::UnregisterTextureView, void*) override;
    void Finalize(nn::gfx::Device*) override;
    void* FindResourceByName(size_t*, u32, const char*) override;
    void FindResourceByType(u32, nn::ui2d::ResourceCallback, void*) const override;
    nn::font::Font* AcquireFont(nn::gfx::Device*, const char*) override;
    nn::ui2d::TextureInfo* AcquireTexture(nn::gfx::Device*, const char*) override;
    nn::ui2d::ShaderInfo* AcquireShader(nn::gfx::Device*, const char*) override;
    nn::ui2d::ShaderInfo* AcquireArchiveShader(nn::gfx::Device*, u32, size_t, const u32*) override;
    bool LoadTexture(nn::ui2d::ResourceTextureInfo*, nn::gfx::Device*, const char*) override;
    bool LoadShader(nn::ui2d::ShaderInfo*, nn::gfx::Device*, const char*) override;
    bool LoadArchiveShader(nn::ui2d::ShaderInfo*, nn::gfx::Device*, u32, size_t, const u32*) override;
    void* findAnimationResource(const char* pLayoutName, const char* pAnimationName, u32* pSize);
    const ArcResourceMgr* mArchives;
    const FontMgr* mFonts;
    nn::util::IntrusiveListNode mFontList;
    nn::util::IntrusiveListNode mShaderList;
    nn::util::IntrusiveListNode mTextureList;
};
static_assert(sizeof(MultiArcResourceAccessor) == 0x48, "MultiArcResourceAccessor size");
}
