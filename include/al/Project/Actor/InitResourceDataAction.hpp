#pragma once

#include <basis/seadTypes.h>

namespace al {
    class InitResourceDataActionAnim;
    class InitResourceDataAnim;
    class Resource;

    /// Action data read from an actor's resource.
    class InitResourceDataAction {
    public:
        static InitResourceDataAction* tryCreate(Resource* pResource, const InitResourceDataAnim* pDataAnim);

        InitResourceDataAction(InitResourceDataActionAnim* pActionAnim);

        InitResourceDataActionAnim* mActionAnim;    // _0
    };
};
