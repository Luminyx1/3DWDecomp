#include <nn/ui2d/ui2d_ArcResourceAccessor.h>
namespace nn::ui2d {
ArchiveHandle::ArchiveHandle() : mTextureFile(nullptr), _B8(nullptr), _C0(nullptr), _C8(nullptr), _D0(nullptr), _D8(nullptr), _E0(nullptr) {}
ArchiveHandle::~ArchiveHandle() = default;
const char* ArchiveHandle::GetResRootDir() const { return mRootDirectory; }
ArcExtractor* ArchiveHandle::GetArcExtractor() { return &mExtractor; }
const ArcExtractor* ArchiveHandle::GetArcExtractor() const { return &mExtractor; }
FontContainer::List* ArchiveHandle::GetFontList() { return &mFonts.mFonts; }
TextureContainer::List* ArchiveHandle::GetTextureList() { return &mTextures.mTextures; }
ShaderContainer::List* ArchiveHandle::GetShaderList() { return &mShaders.mShaders; }
// name identifies font; the archive takes ownership of its resources.
const void* ArchiveHandle::RegisterFont(const char* name, nn::font::Font* font) { return mFonts.RegisterFont(name, font, true); }
// name identifies a new texture wrapper owned by this archive.
ResourceTextureInfo* ArchiveHandle::RegisterTexture(const char* name) { return mTextures.RegisterResourceTexture(name); }
// name identifies a shader backed by shared archive data.
ShaderInfo* ArchiveHandle::RegisterShader(const char* name) { return mShaders.RegisterShader(name, true); }
const void* ArchiveHandle::GetArchiveDataStart() const { return mExtractor.m_pArchiveBlockHeader; }
// callback allocates font and texture descriptors; argument is its context.
void ArchiveHandle::RegisterTextureViewToDescriptorPool(TextureContainer::RegisterCallback callback, void* argument) {
    mFonts.RegisterTextureViewToDescriptorPool(callback, argument);
    mTextures.RegisterTextureViewToDescriptorPool(callback, argument);
}

// callback releases font and texture descriptors; argument is its context.
void ArchiveHandle::UnregisterTextureViewFromDescriptorPool(TextureContainer::UnregisterCallback callback, void* argument) {
    mFonts.UnregisterTextureViewFromDescriptorPool(callback, argument);
    mTextures.UnregisterTextureViewFromDescriptorPool(callback, argument);
}
}
