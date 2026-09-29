#pragma once

#include <basis/seadTypes.h>

namespace al {
    class ActorInitInfo;
    class LiveActor;
    class Resource;

    const u8* getMapPartsResourceYaml(const ActorInitInfo &, const char *);
    bool isExistModelResource(const LiveActor*);
    Resource* getModelResource(const LiveActor*);
    bool isExistModelResourceYaml(const LiveActor*, const char*, const char*);
    const u8* getModelResourceYaml(const LiveActor*, const char*, const char*);
};