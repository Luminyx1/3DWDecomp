#include "Library/Resource/ResourceFunction.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/System/SystemKit.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Resource/ResourceSystem.hpp"

namespace al {
namespace {
inline ResourceSystem* getResourceSystem() {
    return alProjectInterface::getSystemKit()->getResourceSystem();
}

inline const u8* tryGetBymlImpl(const Resource* pResource, const sead::SafeString& rBymlName) {
    StringTmp<128> unused("%s.byml", rBymlName.cstr());
    if (!pResource->isExistFile(StringTmp<128>("%s.byml", rBymlName.cstr()))) {
        return nullptr;
    }

    return pResource->getByml(rBymlName);
}
}  // namespace

/**
 * Adds a resource category to the resource system.
 * @param rName category name
 * @param size maximum resource count
 * @param pHeap heap for the category
 */
void addResourceCategory(const sead::SafeString& rName, s32 size, sead::Heap* pHeap) {
    getResourceSystem()->addCategory(rName, size, pHeap);
}

/**
 * Checks if a resource category was added.
 * @param rName category name
 * @return whether the category exists
 */
bool isCategoryAdded(const sead::SafeString& rName) {
    return getResourceSystem()->isCategoryAdded(rName);
}

/**
 * Checks if a resource category holds no resources.
 * @param rName category name
 * @return whether the category is empty
 */
bool isEmptyCategoryResource(const sead::SafeString& rName) {
    return getResourceSystem()->isEmptyCategoryResource(rName);
}

/**
 * Creates every resource listed for a category.
 * @param rName category name
 * @param pEvent event signaled on completion
 */
void createCategoryResourceAll(const sead::SafeString& rName, sead::Event* pEvent) {
    getResourceSystem()->createCategoryResourceAll(rName, pEvent);
}

/**
 * Removes a resource category.
 * @param rName category name
 */
void removeResourceCategory(const sead::SafeString& rName) {
    getResourceSystem()->removeCategory(rName);
}

/**
 * Gets the archive name of a resource.
 * @param pResource resource
 * @return archive name
 */
const char* getResourceName(const Resource* pResource) {
    return pResource->getArchiveName();
}

/**
 * Checks if a resource has a graphics file.
 * @param pResource resource
 * @return whether the graphics file exists
 */
bool isExistResGraphicsFile(const Resource* pResource) {
    return pResource->getResFile() != nullptr;
}

/**
 * Finds a loaded resource.
 * @param rPath resource path
 * @return resource, or null
 */
Resource* findResource(const sead::SafeString& rPath) {
    return getResourceSystem()->findResource(rPath);
}

/**
 * Finds a resource, creating it if needed.
 * @param rPath resource path
 * @param pExt archive extension
 * @return resource
 */
Resource* findOrCreateResource(const sead::SafeString& rPath, const char* pExt) {
    return getResourceSystem()->findOrCreateResource(rPath, pExt);
}

/**
 * Finds a resource, creating it in the given category if needed.
 * @param rPath resource path
 * @param rCategory category name
 * @param pExt archive extension
 * @return resource
 */
Resource* findOrCreateResourceCategory(const sead::SafeString& rPath,
                                       const sead::SafeString& rCategory, const char* pExt) {
    return getResourceSystem()->findOrCreateResourceCategory(rPath, rCategory, pExt);
}

/**
 * Finds or creates a resource under SystemData.
 * @param pName archive name
 * @param pExt archive extension
 * @return resource
 */
Resource* findOrCreateResourceSystemData(const char* pName, const char* pExt) {
    StringTmp<128> path("SystemData/%s", pName);
    return findOrCreateResource(path, pExt);
}

/**
 * Checks if a resource contains a yaml (byml) file.
 * @param pResource resource
 * @param pName file name
 * @param pSuffix optional file name suffix
 * @return whether the file exists
 */
bool isExistResourceYaml(const Resource* pResource, const char* pName, const char* pSuffix) {
    if (pSuffix) {
        StringTmp<128> fileName;
        createFileNameBySuffix(&fileName, pName, pSuffix);
        return pResource->isExistFile(StringTmp<64>("%s.byml", fileName.cstr()));
    }

    return pResource->isExistFile(StringTmp<64>("%s.byml", pName));
}

/**
 * Gets a yaml (byml) file from a resource.
 * @param pResource resource
 * @param pName file name
 * @param pSuffix optional file name suffix
 * @return byml data
 */
const u8* findResourceYaml(const Resource* pResource, const char* pName, const char* pSuffix) {
    if (pSuffix) {
        StringTmp<128> fileName;
        createFileNameBySuffix(&fileName, pName, pSuffix);
        pName = fileName.cstr();
    }

    return pResource->getByml(pName);
}

/**
 * Gets a file from a stage's design archive if it exists.
 * @param rStageName stage name
 * @param rFileName file name
 * @param scenarioNo scenario number
 * @return file data, or null
 */
const void* tryFindStageParameterFileDesign(const sead::SafeString& rStageName,
                                            const sead::SafeString& rFileName, s32 scenarioNo) {
    StringTmp<128> path("StageData/%sDesign", rStageName.cstr());
    if (!isExistArchive(path)) {
        path.appendWithFormat("%d", scenarioNo);
        if (!isExistArchive(path)) {
            return nullptr;
        }
    }

    Resource* resource = findOrCreateResource(path, nullptr);
    if (!resource->isExistFile(rFileName)) {
        return nullptr;
    }

    return resource->getOtherFile(rFileName, nullptr);
}

/**
 * Gets the file list iterator of a category.
 * @param pIter output iterator
 * @param rCategory category name
 * @return whether the category has a file list
 */
bool tryGetCategoryFileListIter(ByamlIter* pIter, const sead::SafeString& rCategory) {
    return getResourceSystem()->tryGetCategoryFileListIter(pIter, rCategory);
}

/**
 * Sets the category new resources are created in.
 * @param pName category name
 */
void setCurrentCategoryName(const char* pName) {
    getResourceSystem()->setCurrentCategory(pName);
}

/**
 * Gets a byml file from an ObjectData archive.
 * @param rObjectName object archive name
 * @param rBymlName byml file name
 * @return byml data
 */
const u8* getBymlFromObjectResource(const sead::SafeString& rObjectName,
                                    const sead::SafeString& rBymlName) {
    StringTmp<256> path("ObjectData/%s", rObjectName.cstr());
    return findOrCreateResource(path, nullptr)->getByml(rBymlName);
}

/**
 * Gets a byml file from an ObjectData archive if it exists.
 * @param rObjectName object archive name
 * @param rBymlName byml file name
 * @return byml data, or null
 */
const u8* tryGetBymlFromObjectResource(const sead::SafeString& rObjectName,
                                       const sead::SafeString& rBymlName) {
    StringTmp<256> path("ObjectData/%s", rObjectName.cstr());
    Resource* resource = findOrCreateResource(path, nullptr);
    StringTmp<128> unused("%s.byml", rBymlName.cstr());
    if (!resource->isExistFile(StringTmp<128>("%s.byml", rBymlName.cstr()))) {
        return nullptr;
    }

    return resource->getByml(rBymlName);
}

/**
 * Gets a byml file from a LayoutData archive.
 * @param rLayoutName layout archive name
 * @param rBymlName byml file name
 * @return byml data
 */
const u8* getBymlFromLayoutResource(const sead::SafeString& rLayoutName,
                                    const sead::SafeString& rBymlName) {
    StringTmp<256> path("LayoutData/%s", rLayoutName.cstr());
    return findOrCreateResource(path, nullptr)->getByml(rBymlName);
}

/**
 * Gets a byml file from a LayoutData archive if it exists.
 * @param rLayoutName layout archive name
 * @param rBymlName byml file name
 * @return byml data, or null
 */
const u8* tryGetBymlFromLayoutResource(const sead::SafeString& rLayoutName,
                                       const sead::SafeString& rBymlName) {
    StringTmp<256> path("LayoutData/%s", rLayoutName.cstr());
    Resource* resource = findOrCreateResource(path, nullptr);
    StringTmp<128> unused("%s.byml", rBymlName.cstr());
    if (!resource->isExistFile(StringTmp<128>("%s.byml", rBymlName.cstr()))) {
        return nullptr;
    }

    return resource->getByml(rBymlName);
}

/**
 * Gets a byml file from an archive if it exists.
 * @param rArchiveName archive path
 * @param rBymlName byml file name
 * @return byml data, or null
 */
const u8* tryGetBymlFromArcName(const sead::SafeString& rArchiveName,
                                const sead::SafeString& rBymlName) {
    return tryGetBymlImpl(findOrCreateResource(rArchiveName, nullptr), rBymlName);
}

/**
 * Gets a byml file from a resource if it exists.
 * @param pResource resource
 * @param rBymlName byml file name
 * @return byml data, or null
 */
const u8* tryGetByml(const Resource* pResource, const sead::SafeString& rBymlName) {
    return tryGetBymlImpl(pResource, rBymlName);
}
}  // namespace al
