#include "Project/Actor/InitResourceDataAction.hpp"
#include "Project/Actor/InitResourceDataActionAnim.hpp"

namespace al {
    /**
     * @brief Creates the action data if the resource has action animation data.
     * @param pResource The actor's resource.
     * @param pDataAnim The actor's animation data.
     * @return The created action data, or nullptr if there is none.
     */
    InitResourceDataAction* InitResourceDataAction::tryCreate(Resource* pResource, const InitResourceDataAnim* pDataAnim) {
        InitResourceDataActionAnim* actionAnim = InitResourceDataActionAnim::tryCreate(pResource, pDataAnim);
        if (actionAnim == nullptr) {
            return nullptr;
        }
        return new InitResourceDataAction(actionAnim);
    }

    /**
     * @brief Constructs the action data.
     * @param pActionAnim The action animation data.
     */
    InitResourceDataAction::InitResourceDataAction(InitResourceDataActionAnim* pActionAnim) : mActionAnim(pActionAnim) {}
};
