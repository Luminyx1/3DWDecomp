#pragma once

#include <eui/euiMultiArcResourceAccessor.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::gfx {
class ResTextureFile;
}

namespace al {
class LayoutSystem;
class Resource;

class LayoutResource : public eui::MultiArcResourceAccessor {
public:
    struct TextureFileLink {
        TextureFileLink(nn::gfx::ResTextureFile* pTextureFile) : mTextureFile(pTextureFile) {}

        nn::util::IntrusiveListNode mLink;
        nn::gfx::ResTextureFile* mTextureFile;
    };

    LayoutResource(const Resource* pResource, const LayoutSystem* pLayoutSystem,
                   bool isLocalized);
    ~LayoutResource() override;

    void addResourceLink(const Resource* pResource);
    void reinitializeShaders(nn::gfx::Device* pDevice);
    void loadAllTextures(nn::gfx::Device* pDevice);
    void* FindResourceByName(size_t* pSize, u32 type, const char* pName) override;
    void Finalize(nn::gfx::Device* pDevice) override;
    nn::font::Font* AcquireFont(nn::gfx::Device* pDevice, const char* pName) override;

    bool isLocalized() const { return mIsLocalized; }

private:
    bool mIsLocalized;
    nn::util::IntrusiveListNode mTextureFiles;
};

static_assert(sizeof(LayoutResource) == 0x60);
}  // namespace al
