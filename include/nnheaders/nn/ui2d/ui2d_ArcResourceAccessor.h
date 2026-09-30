#pragma once
#include <nn/ui2d/ui2d_ArcExtractor.h>
#include <nn/ui2d/ui2d_FontContainer.h>
#include <nn/ui2d/ui2d_TextureContainer.h>
#include <nn/ui2d/ui2d_ShaderContainer.h>
namespace nn::ui2d {
class ArchiveHandle {
public:
    ArchiveHandle();
    virtual ~ArchiveHandle();
    const char* GetResRootDir() const;
    ArcExtractor* GetArcExtractor();
    const ArcExtractor* GetArcExtractor() const;
    FontContainer::List* GetFontList();
    TextureContainer::List* GetTextureList();
    ShaderContainer::List* GetShaderList();
    const void* RegisterFont(const char* name, nn::font::Font* font);
    ResourceTextureInfo* RegisterTexture(const char* name);
    ShaderInfo* RegisterShader(const char* name);
    const void* GetArchiveDataStart() const;
    void RegisterTextureViewToDescriptorPool(TextureContainer::RegisterCallback callback, void* argument);
    void UnregisterTextureViewFromDescriptorPool(TextureContainer::UnregisterCallback callback, void* argument);
    ArcExtractor mExtractor;
    char mRootDirectory[64];
    FontContainer mFonts;
    TextureContainer mTextures;
    ShaderContainer mShaders;
    void* mTextureFile;
    void* _B8;
    void* _C0;
    void* _C8;
    void* _D0;
    void* _D8;
    void* _E0;
};
static_assert(sizeof(ArchiveHandle) == 0xe8, "ArchiveHandle size");
}
