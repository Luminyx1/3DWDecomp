#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ActionAnimCtrlInfo;
class InitResourceDataAnim;
class Resource;

class InitResourceDataActionAnim {
public:
    static InitResourceDataActionAnim* tryCreate(Resource* pResource,
                                                 const InitResourceDataAnim* pDataAnim);

    InitResourceDataActionAnim(Resource* pResource, const InitResourceDataAnim* pDataAnim);
    void sortCtrlInfo();

    s32 getAnimInfoCount() const { return mAnimInfoCount; }
    ActionAnimCtrlInfo** getAnimInfos() const { return mAnimInfos; }

private:
    s32 mAnimInfoCount = 0;
    ActionAnimCtrlInfo** mAnimInfos = nullptr;
};

class InitResourceDataAction {
public:
    static InitResourceDataAction* tryCreate(Resource* pResource,
                                             const InitResourceDataAnim* pDataAnim);

    InitResourceDataAction(InitResourceDataActionAnim* pDataActionAnim);

    InitResourceDataActionAnim* getDataActionAnim() const { return mDataActionAnim; }

private:
    InitResourceDataActionAnim* mDataActionAnim;
};

}  // namespace al
