#pragma once

namespace agl::fx {
class OfxLensFlareDynamic;
}

namespace al {

class OccludedEffectRequestInfo {
public:
    OccludedEffectRequestInfo(agl::fx::OfxLensFlareDynamic* pLensFlare);

private:
    agl::fx::OfxLensFlareDynamic* mLensFlare;
};

}  // namespace al
