#pragma once

#include <eui/euiMultiArcResourceAccessor.h>

namespace al {
class LayoutSystem;
class Resource;

class LayoutResource : public eui::MultiArcResourceAccessor {
public:
    LayoutResource(const Resource* pResource, const LayoutSystem* pLayoutSystem, bool isLoadTexture);
    ~LayoutResource() override;

    void addResourceLink(const Resource* pResource);
    void reinitializeShaders(nn::gfx::Device* pDevice);
    void loadAllTextures(nn::gfx::Device* pDevice);
};
}  // namespace al
