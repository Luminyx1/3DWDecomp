#include "Project/Actor/ActorParamHolder.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"

namespace al {
    /**
     * @brief Creates the parameter holder if the actor's model has an ActorParam resource.
     * @param pActor The actor to read the parameters of.
     * @return The created holder, or nullptr if the actor has no parameters.
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
};
