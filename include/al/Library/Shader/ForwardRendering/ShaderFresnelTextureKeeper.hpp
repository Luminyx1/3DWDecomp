#pragma once

#include <basis/seadTypes.h>

namespace agl {
class SamplerLocation;
}

namespace al {

class ShaderFresnelTextureKeeper {
public:
    void activateSilhouetteCurveTexture(s32 category, const agl::SamplerLocation& rLocation,
                                        bool isForce) const;
};

}  // namespace al
