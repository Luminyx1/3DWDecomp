#pragma once

#include <basis/seadTypes.h>

#include "Library/Play/Draw/PartsGraphics.hpp"

namespace agl {
class TextureSampler;
}  // namespace agl

namespace al {
class GraphicsSystemInfo;
class ShaderHolder;

class NoiseTextureKeeper : public PartsGraphics {
public:
    NoiseTextureKeeper(GraphicsSystemInfo* pInfo, ShaderHolder* pShaderHolder);
    ~NoiseTextureKeeper();

    void finalize() override;
    void endInit() override;
    void drawSystem(const GraphicsRenderInfo* pInfo) const override;
    void update(const GraphicsUpdateInfo& rInfo) override;
    void calcGpu(const GraphicsCalcGpuInfo& rInfo) override;
    const char* getName() const override;

    const agl::TextureSampler* getTexture3DSampler(s32 index) const;

private:
    u8 _28[0x248 - 0x28];
};

static_assert(sizeof(NoiseTextureKeeper) == 0x248);

}  // namespace al
