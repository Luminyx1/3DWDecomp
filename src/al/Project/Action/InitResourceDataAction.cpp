#include "Project/Action/Common/InitResourceDataAction.hpp"

namespace al {

/**
 * Creates action data if the resource has an action animation table.
 * @param pResource Resource to read from.
 * @param pDataAnim Animation data of the resource.
 * @return Created data, or nullptr.
 */
InitResourceDataAction* InitResourceDataAction::tryCreate(Resource* pResource,
                                                          const InitResourceDataAnim* pDataAnim) {
    InitResourceDataActionAnim* dataActionAnim =
        InitResourceDataActionAnim::tryCreate(pResource, pDataAnim);
    if (dataActionAnim == nullptr) {
        return nullptr;
    }

    return new InitResourceDataAction(dataActionAnim);
}

/**
 * Constructs action data from an action animation table.
 * @param pDataActionAnim Action animation table.
 */
InitResourceDataAction::InitResourceDataAction(InitResourceDataActionAnim* pDataActionAnim)
    : mDataActionAnim(pDataActionAnim) {}

}  // namespace al
