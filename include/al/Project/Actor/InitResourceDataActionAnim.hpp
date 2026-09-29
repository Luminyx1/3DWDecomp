#pragma once

#include <basis/seadTypes.h>

namespace al {
    class InitResourceDataAnim;
    class Resource;

    /// Action animation data read from an actor's resource.
    class InitResourceDataActionAnim {
    public:
        static InitResourceDataActionAnim* tryCreate(Resource* pResource, const InitResourceDataAnim* pDataAnim);

        InitResourceDataActionAnim(Resource* pResource, const InitResourceDataAnim* pDataAnim);
    };
};
