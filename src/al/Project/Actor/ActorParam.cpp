#include "Library/LiveActor/ActorParamHolder.hpp"

#include "Library/LiveActor/Util/ActorResourceUtil.hpp"

namespace al {
/**
 * Constructs an empty actor parameter.
 */
ActorParamInfo::ActorParamInfo() = default;

/**
 * Creates a parameter holder if the actor's model has an ActorParam file.
 * @param pActor The actor.
 * @return The new holder, or nullptr.
 */
ActorParamHolder* ActorParamHolder::tryCreate(LiveActor* pActor) {
    if (!isExistModelResource(pActor)) {
        return nullptr;
    }
    if (!isExistModelResourceYaml(pActor, "ActorParam", nullptr)) {
        return nullptr;
    }
    return new ActorParamHolder(pActor);
}
}  // namespace al
