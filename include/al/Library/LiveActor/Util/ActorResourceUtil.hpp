#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
    class ActorInitInfo;
    class ByamlIter;
    class LiveActor;
    class Resource;

    bool isExistModelResource(const LiveActor*);
    bool isExistAnimResource(const LiveActor*);
    Resource* tryGetAnimResource(const LiveActor*);
    bool isExistModelResourceYaml(const LiveActor*, const char*, const char*);
    Resource* getModelResource(const LiveActor*);
    bool isExistAnimResourceYaml(const LiveActor*, const char*, const char*);
    const u8* getModelResourceYaml(const LiveActor*, const char*, const char*);
    const u8* getAnimResourceYaml(const LiveActor*, const char*, const char*);
    const u8* getMapPartsResourceYaml(const ActorInitInfo &, const char *);
    const u8* tryGetMapPartsResourceYaml(const ActorInitInfo&, const char*);
    Resource* getAnimResource(const LiveActor*);
    bool isExistModelOrAnimResourceYaml(const LiveActor*, const char*, const char*);
    const u8* getModelOrAnimResourceYaml(const LiveActor*, const char*, const char*);
    bool tryGetActorInitFileIter(ByamlIter*, const LiveActor*, const char*, const char*);

    bool tryGetInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                   const Resource* pResource, const char* pKey,
                                   const char* pSuffix, const char* pInitName,
                                   const Resource* pSubResource);
    bool tryGetSuffixIter(ByamlIter* pIter, const Resource* pResource, const char* pName,
                          const char* pSuffix);
    bool tryGetActorInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                        const Resource* pResource, const char* pKey,
                                        const char* pSuffix);
    bool tryGetLayoutActorInitFileIterAndName(ByamlIter* pIter,
                                              sead::BufferedSafeString* pFileName,
                                              const Resource* pResource, const char* pKey,
                                              const char* pSuffix);
    bool tryGetActorInitFileIter(ByamlIter* pIter, const Resource* pResource, const char* pKey,
                                 const char* pSuffix, const char* pInitName);
    bool tryGetActorInitFileIter(ByamlIter* pIter, const Resource* pResource, const char* pKey,
                                 const char* pSuffix);
    bool tryGetLayoutActorInitFileIter(ByamlIter* pIter, const Resource* pResource,
                                       const char* pKey, const char* pSuffix);
    bool tryGetActorInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                        const LiveActor* pActor, const char* pKey,
                                        const char* pSuffix);
    bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const Resource* pResource,
                                 const char* pKey, const char* pSuffix);
    bool tryGetLayoutActorInitFileName(sead::BufferedSafeString* pFileName,
                                       const Resource* pResource, const char* pKey,
                                       const char* pSuffix);
    bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const Resource* pResource,
                                 const char* pKey, const char* pSuffix, const char* pInitName);
    bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const LiveActor* pActor,
                                 const char* pKey, const char* pSuffix);
    bool tryGetActorAnimInitFileName(sead::BufferedSafeString* pFileName, const LiveActor* pActor,
                                     const char* pKey, const char* pSuffix);
};
