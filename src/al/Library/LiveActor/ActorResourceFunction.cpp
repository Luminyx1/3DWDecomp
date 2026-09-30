#include "Library/LiveActor/Util/ActorResourceUtil.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Checks whether an actor has a model resource.
 * @param pActor The actor.
 * @return Whether the model resource exists.
 */
bool isExistModelResource(const LiveActor* pActor) {
    return pActor->mModelKeeper != nullptr;
}

/**
 * Checks whether an actor has an animation resource.
 * @param pActor The actor.
 * @return Whether the animation resource exists.
 */
bool isExistAnimResource(const LiveActor* pActor) {
    return tryGetAnimResource(pActor) != nullptr;
}

/**
 * Gets the animation resource of an actor, if any.
 * @param pActor The actor.
 * @return The animation resource or nullptr.
 */
Resource* tryGetAnimResource(const LiveActor* pActor) {
    return const_cast<Resource*>(pActor->mModelKeeper->mModelCafe->getAnimResource());
}

/**
 * Checks whether a yaml file exists in an actor's model resource.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return Whether the file exists.
 */
bool isExistModelResourceYaml(const LiveActor* pActor, const char* pName, const char* pSuffix) {
    return isExistResourceYaml(getModelResource(pActor), pName, pSuffix);
}

/**
 * Gets the model resource of an actor.
 * @param pActor The actor.
 * @return The model resource.
 */
Resource* getModelResource(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelCafe->mModelRes;
}

/**
 * Checks whether a yaml file exists in an actor's animation resource.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return Whether the file exists.
 */
bool isExistAnimResourceYaml(const LiveActor* pActor, const char* pName, const char* pSuffix) {
    return isExistResourceYaml(getAnimResource(pActor), pName, pSuffix);
}

/**
 * Gets the animation resource of an actor.
 * @param pActor The actor.
 * @return The animation resource.
 */
Resource* getAnimResource(const LiveActor* pActor) {
    return const_cast<Resource*>(pActor->mModelKeeper->mModelCafe->getAnimResource());
}

/**
 * Checks whether a yaml file exists in an actor's model or animation resource.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return Whether the file exists.
 */
bool isExistModelOrAnimResourceYaml(const LiveActor* pActor, const char* pName,
                                    const char* pSuffix) {
    if (isExistModelResourceYaml(pActor, pName, pSuffix)) {
        return true;
    }
    return tryGetAnimResource(pActor) && isExistResourceYaml(getAnimResource(pActor), pName, pSuffix);
}

/**
 * Gets a yaml file from an actor's model resource.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return The yaml data.
 */
const u8* getModelResourceYaml(const LiveActor* pActor, const char* pName, const char* pSuffix) {
    return findResourceYaml(getModelResource(pActor), pName, pSuffix);
}

/**
 * Gets a yaml file from an actor's animation resource.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return The yaml data.
 */
const u8* getAnimResourceYaml(const LiveActor* pActor, const char* pName, const char* pSuffix) {
    return findResourceYaml(getAnimResource(pActor), pName, pSuffix);
}

/**
 * Gets a yaml file from an actor's model resource, or from its animation resource if missing.
 * @param pActor The actor.
 * @param pName The file name.
 * @param pSuffix The file suffix.
 * @return The yaml data.
 */
const u8* getModelOrAnimResourceYaml(const LiveActor* pActor, const char* pName,
                                     const char* pSuffix) {
    if (isExistModelResourceYaml(pActor, pName, pSuffix)) {
        return getModelResourceYaml(pActor, pName, pSuffix);
    }
    return getAnimResourceYaml(pActor, pName, pSuffix);
}

/**
 * Gets a yaml file from the map parts resource of a placement.
 * @param rInfo The actor init info.
 * @param pName The file name.
 * @return The yaml data.
 */
const u8* getMapPartsResourceYaml(const ActorInitInfo& rInfo, const char* pName) {
    StringTmp<256> modelName;
    StringTmp<256> path;
    makeMapPartsModelName(&modelName, &path, *rInfo.mPlacementInfo);
    return findOrCreateResource(path, nullptr)->getByml(pName);
}

/**
 * Gets a yaml file from the map parts resource of a placement, if it exists.
 * @param rInfo The actor init info.
 * @param pName The file name.
 * @return The yaml data or nullptr.
 */
const u8* tryGetMapPartsResourceYaml(const ActorInitInfo& rInfo, const char* pName) {
    StringTmp<256> modelName;
    StringTmp<256> path;
    makeMapPartsModelName(&modelName, &path, *rInfo.mPlacementInfo);
    Resource* resource = findOrCreateResource(path, nullptr);
    StringTmp<256> fileName("%s.byml", pName);
    if (!resource->isExistFile(StringTmp<256>("%s.byml", pName))) {
        return nullptr;
    }
    return resource->getByml(pName);
}

/**
 * Gets an init file iterator and its name, taking the file name from a suffix file if present.
 * @param pIter The output iterator, may be nullptr.
 * @param pFileName The output file name, may be nullptr.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @param pInitName The name of the suffix file.
 * @param pSubResource A second resource to search, may be nullptr.
 * @return Whether the file was found.
 */
bool tryGetInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                               const Resource* pResource, const char* pKey, const char* pSuffix,
                               const char* pInitName, const Resource* pSubResource) {
    const char* suffixName = nullptr;
    ByamlIter suffixIter;
    if (tryGetSuffixIter(&suffixIter, pResource, pInitName, pSuffix)) {
        if (!suffixIter.isExistKey(pKey)) {
            return false;
        }
        suffixIter.tryGetStringByKey(&suffixName, pKey);
    }

    StringTmp<256> fileName;
    createFileNameBySuffix(&fileName, pKey, suffixName);
    StringTmp<64> filePath("%s.byml", fileName.cstr());

    if (pResource->isExistFile(filePath)) {
        if (pIter) {
            *pIter = ByamlIter(pResource->getByml(fileName));
        }
    } else if (pSubResource && pSubResource->isExistFile(filePath)) {
        if (pIter) {
            *pIter = ByamlIter(pSubResource->getByml(fileName));
        }
    } else {
        return false;
    }
    if (pFileName) {
        pFileName->format(fileName.cstr());
    }
    return true;
}

/**
 * Gets the iterator of a suffix file.
 * @param pIter The output iterator.
 * @param pResource The resource.
 * @param pName The base file name.
 * @param pSuffix The suffix.
 * @return Whether the suffix file exists.
 */
bool tryGetSuffixIter(ByamlIter* pIter, const Resource* pResource, const char* pName,
                      const char* pSuffix) {
    if (!pSuffix) {
        return false;
    }
    StringTmp<256> fileName;
    createFileNameBySuffix(&fileName, pName, pSuffix);
    if (!pResource->isExistFile(StringTmp<64>("%s.byml", fileName.cstr()))) {
        return false;
    }
    *pIter = ByamlIter(pResource->getByml(fileName));
    return true;
}

/**
 * Gets an actor init file iterator and its name.
 * @param pIter The output iterator.
 * @param pFileName The output file name.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                    const Resource* pResource, const char* pKey,
                                    const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, pFileName, pResource, pKey, pSuffix, "InitActor",
                                     nullptr);
}

/**
 * Gets a layout actor init file iterator and its name.
 * @param pIter The output iterator.
 * @param pFileName The output file name.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetLayoutActorInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                          const Resource* pResource, const char* pKey,
                                          const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, pFileName, pResource, pKey, pSuffix,
                                     "InitLayoutActor", nullptr);
}

/**
 * Gets an init file iterator using a custom suffix file name.
 * @param pIter The output iterator.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @param pInitName The name of the suffix file.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileIter(ByamlIter* pIter, const Resource* pResource, const char* pKey,
                             const char* pSuffix, const char* pInitName) {
    return tryGetInitFileIterAndName(pIter, nullptr, pResource, pKey, pSuffix, pInitName, nullptr);
}

/**
 * Gets an actor init file iterator.
 * @param pIter The output iterator.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileIter(ByamlIter* pIter, const Resource* pResource, const char* pKey,
                             const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, nullptr, pResource, pKey, pSuffix, "InitActor",
                                     nullptr);
}

/**
 * Gets a layout actor init file iterator.
 * @param pIter The output iterator.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetLayoutActorInitFileIter(ByamlIter* pIter, const Resource* pResource, const char* pKey,
                                   const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, nullptr, pResource, pKey, pSuffix, "InitLayoutActor",
                                     nullptr);
}

/**
 * Gets an actor init file iterator and its name from an actor's model resource.
 * @param pIter The output iterator.
 * @param pFileName The output file name.
 * @param pActor The actor.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileIterAndName(ByamlIter* pIter, sead::BufferedSafeString* pFileName,
                                    const LiveActor* pActor, const char* pKey,
                                    const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, pFileName, getModelResource(pActor), pKey, pSuffix,
                                     "InitActor", nullptr);
}

/**
 * Gets an actor init file iterator from an actor's model resource.
 * @param pIter The output iterator.
 * @param pActor The actor.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileIter(ByamlIter* pIter, const LiveActor* pActor, const char* pKey,
                             const char* pSuffix) {
    return tryGetInitFileIterAndName(pIter, nullptr, getModelResource(pActor), pKey, pSuffix,
                                     "InitActor", nullptr);
}

/**
 * Gets the name of an actor init file.
 * @param pFileName The output file name.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const Resource* pResource,
                             const char* pKey, const char* pSuffix) {
    return tryGetInitFileIterAndName(nullptr, pFileName, pResource, pKey, pSuffix, "InitActor",
                                     nullptr);
}

/**
 * Gets the name of a layout actor init file.
 * @param pFileName The output file name.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetLayoutActorInitFileName(sead::BufferedSafeString* pFileName,
                                   const Resource* pResource, const char* pKey,
                                   const char* pSuffix) {
    return tryGetInitFileIterAndName(nullptr, pFileName, pResource, pKey, pSuffix,
                                     "InitLayoutActor", nullptr);
}

/**
 * Gets the name of an init file using a custom suffix file name.
 * @param pFileName The output file name.
 * @param pResource The resource.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @param pInitName The name of the suffix file.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const Resource* pResource,
                             const char* pKey, const char* pSuffix, const char* pInitName) {
    return tryGetInitFileIterAndName(nullptr, pFileName, pResource, pKey, pSuffix, pInitName,
                                     nullptr);
}

/**
 * Gets the name of an actor init file from an actor's model resource.
 * @param pFileName The output file name.
 * @param pActor The actor.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorInitFileName(sead::BufferedSafeString* pFileName, const LiveActor* pActor,
                             const char* pKey, const char* pSuffix) {
    return tryGetInitFileIterAndName(nullptr, pFileName, getModelResource(pActor), pKey, pSuffix,
                                     "InitActor", nullptr);
}

/**
 * Gets the name of an actor init file, also searching the actor's animation resource.
 * @param pFileName The output file name.
 * @param pActor The actor.
 * @param pKey The init file key.
 * @param pSuffix The suffix.
 * @return Whether the file was found.
 */
bool tryGetActorAnimInitFileName(sead::BufferedSafeString* pFileName, const LiveActor* pActor,
                                 const char* pKey, const char* pSuffix) {
    const Resource* animResource = tryGetAnimResource(pActor);
    if (!animResource) {
        return false;
    }
    return tryGetInitFileIterAndName(nullptr, pFileName, getModelResource(pActor), pKey, pSuffix,
                                     "InitActor", animResource);
}
}  // namespace al
