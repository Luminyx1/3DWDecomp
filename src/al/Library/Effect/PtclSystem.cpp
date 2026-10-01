#include "Library/Effect/PtclSystem.hpp"

#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"

namespace al {

/**
 * Constructs the particle system of an effect system.
 * @param rConfig Particle system configuration.
 * @param pEffectSystem Owning effect system.
 */
PtclSystem::PtclSystem(const sead::ptcl::Config& rConfig, EffectSystem* pEffectSystem)
    : sead::ptcl::PtclSystem(rConfig), mEffectSystem(pEffectSystem) {
    mEmitterSetResourceInfoHolder = new EmitterSetResourceInfoHolder();
}

/**
 * Returns the number of particle resources.
 * @return Number of resources.
 */
s32 PtclSystem::getNumResource() const {
    return GetResourceNum();
}

/**
 * Builds the emitter set database after all resources were entered.
 */
void PtclSystem::entryResourceEnd() {
    mEmitterSetResourceInfoHolder->createDataBase(this);
}

/**
 * Returns the environment parameters of the owning effect system.
 * @return Environment parameters.
 */
EffectEnvParam* PtclSystem::getEffectEnvParam() {
    return mEffectSystem->getEffectEnvParam();
}

}  // namespace al
