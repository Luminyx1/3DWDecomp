#include "Library/PostProcessing/OccludedEffectRequestInfo.hpp"

namespace al {

/**
 * Constructs a request info for a lens flare.
 * @param pLensFlare Lens flare.
 */
OccludedEffectRequestInfo::OccludedEffectRequestInfo(agl::fx::OfxLensFlareDynamic* pLensFlare)
    : mLensFlare(pLensFlare) {}

}  // namespace al
