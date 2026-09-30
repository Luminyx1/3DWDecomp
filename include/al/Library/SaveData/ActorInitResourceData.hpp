#pragma once

namespace al {
class Resource;
class InitResourceDataAnim;
class InitResourceDataAction;

class ActorInitResourceData {
public:
    ActorInitResourceData(Resource* pResource);

    InitResourceDataAnim* getAnimData() const { return mResDataAnim; }
    InitResourceDataAction* getDataAction() const { return mResDataAction; }

    Resource* mResource;
    InitResourceDataAnim* mResDataAnim = nullptr;
    InitResourceDataAction* mResDataAction = nullptr;
};
}  // namespace al
