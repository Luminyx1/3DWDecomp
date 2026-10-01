#pragma once

#include <basis/seadTypes.h>
#include <ptcl/seadPtclSystem.h>

namespace al {
class EffectEnvParam;
class EffectSystem;
class EmitterSetResourceInfoHolder;

class PtclSystem : public sead::ptcl::PtclSystem {
public:
    PtclSystem(const sead::ptcl::Config& rConfig, EffectSystem* pEffectSystem);

    s32 getNumResource() const;
    void entryResourceEnd();
    EffectEnvParam* getEffectEnvParam();

    EffectSystem* getEffectSystem() const { return mEffectSystem; }
    EmitterSetResourceInfoHolder* getEmitterSetResourceInfoHolder() const {
        return mEmitterSetResourceInfoHolder;
    }

private:
    EffectSystem* mEffectSystem;
    EmitterSetResourceInfoHolder* mEmitterSetResourceInfoHolder;
};

static_assert(sizeof(PtclSystem) == 0x2928);
}  // namespace al
