#pragma once

#include <basis/seadTypes.h>

namespace agl {
class TextureSampler;
}  // namespace agl

namespace al {

class NoiseTextureKeeper {
public:
    const agl::TextureSampler* getTexture3DSampler(s32 index) const;
};

}  // namespace al
