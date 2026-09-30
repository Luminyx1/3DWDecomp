#include "Library/SaveData/ActorInitResourceData.hpp"

namespace al {
class InitResourceDataAnim {
public:
    static InitResourceDataAnim* tryCreate(Resource* pResource, Resource* pAnimResource,
                                           Resource* pExtResource);
};

class InitResourceDataAction {
public:
    static InitResourceDataAction* tryCreate(Resource* pResource,
                                             const InitResourceDataAnim* pAnimData);
};

/**
 * Creates the anim and action init data for an actor resource.
 * @param pResource The actor's model resource.
 */
ActorInitResourceData::ActorInitResourceData(Resource* pResource) : mResource(pResource) {
    mResDataAnim = InitResourceDataAnim::tryCreate(pResource, nullptr, nullptr);
    mResDataAction = InitResourceDataAction::tryCreate(pResource, mResDataAnim);
}
}  // namespace al
