#pragma once

#include <basis/seadTypes.h>

namespace al {
class EffectSystem;
class EmitterSetResourceInfoHolder;

// TODO: derives from sead::ptcl::PtclSystem (no header yet)
class PtclSystem {
public:
    EffectSystem* getEffectSystem() const { return mEffectSystem; }
    EmitterSetResourceInfoHolder* getEmitterSetResourceInfoHolder() const {
        return mEmitterSetResourceInfoHolder;
    }

    u8 _0[0x2918];
    EffectSystem* mEffectSystem;
    EmitterSetResourceInfoHolder* mEmitterSetResourceInfoHolder;
};
}  // namespace al
