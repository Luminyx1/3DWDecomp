#include "Project/Resource/ResourceSystem.hpp"

#include <heap/seadHeapMgr.h>
#include <thread/seadEvent.h>

#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/SaveData/ActorInitResourceData.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
void createResourceCore(ResourceSystem* pSystem, Resource* pResource, const char* pArchiveName,
                        Resource* pParent);

namespace {
void cleanupResGraphicsFile(sead::SafeString& rKey, Resource* pResource) {
    pResource->cleanupResGraphicsFile();
}

class ResourceAudio {
public:
    ResourceAudio(ResourceSystem::ResourceAudioInfo* pInfo) : mInfo(pInfo) {}

    void disableSoundMemoryPoolHandler(sead::TreeMapImpl<sead::SafeString>::Node* pNode) {
        ResourceSystem::ResourceAudioInfo* info = mInfo;
        if (pNode->key().comparen(info->mFilePath, info->mFilePath.calcLength()) != 0) {
            return;
        }

        SeadAudioPlayer* player = alAudioSystemFunction::tryFindAudioPlayerRegistedSoundMemoryPoolHandler(
            pNode->key().cstr(), info->mAudioPlayerA, info->mAudioPlayerB);
        if (player) {
            while (!alAudioSystemFunction::tryDisableSoundMemoryPoolHandlerByFilePath(
                pNode->key().cstr(), player)) {
            }
        }
    }

private:
    ResourceSystem::ResourceAudioInfo* mInfo;
};
}  // namespace

/**
 * Constructs the resource system and loads the resource category table.
 * @param pArchivePath resource system archive path, or null for the default one
 */
ResourceSystem::ResourceSystem(const char* pArchivePath) {
    addCategory("リソースシステム", 1, sead::HeapMgr::instance()->getCurrentHeap());
    const char* archivePath = "SystemData/ResourceSystem";
    if (pArchivePath) {
        archivePath = pArchivePath;
    }

    if (isExistArchive(archivePath)) {
        Resource* resource = findOrCreateResourceCategory(archivePath, "リソースシステム", nullptr);
        if (resource) {
            mResourceCategoryTable = new ByamlIter(resource->getByml("ResourceCategoryTable"));
        }
    }
}

/**
 * Adds a resource category if it doesn't exist yet.
 * @param rName category name
 * @param size maximum resource count
 * @param pHeap heap of the category
 * @return category
 */
ResourceSystem::ResourceCategory* ResourceSystem::addCategory(const sead::SafeString& rName,
                                                              s32 size, sead::Heap* pHeap) {
    auto iter = findResourceCategoryIter(rName);
    if (iter != mCategories.end()) {
        return *iter;
    }

    sead::ScopedCurrentHeapSetter setter(pHeap);
    ResourceCategory* category = new ResourceCategory(rName, pHeap);
    category->mResources.allocBuffer(size, nullptr);
    mCategories.pushBack(category);
    return category;
}

/**
 * Finds a resource, creating it in the given category if needed.
 * @param rPath resource path
 * @param rCategory category name
 * @param pExt archive extension
 * @return resource, or null
 */
Resource* ResourceSystem::findOrCreateResourceCategory(const sead::SafeString& rPath,
                                                       const sead::SafeString& rCategory,
                                                       const char* pExt) {
    Resource* resource = findResource(rPath);
    if (resource) {
        return resource;
    }

    auto iter = findResourceCategoryIter(rCategory);
    if (iter == mCategories.end()) {
        return nullptr;
    }

    return createResource(rPath, *iter, pExt);
}

/**
 * Finds a category by name.
 * @param rName category name
 * @return category iterator
 */
sead::RingBuffer<ResourceSystem::ResourceCategory*>::iterator
ResourceSystem::findResourceCategoryIter(const sead::SafeString& rName) {
    for (auto iter = mCategories.begin(); iter != mCategories.end(); ++iter) {
        if (isEqualString((*iter)->mName.cstr(), rName.cstr())) {
            return iter;
        }
    }

    return mCategories.end();
}

/**
 * Checks if a category was added.
 * @param rName category name
 * @return whether the category exists
 */
bool ResourceSystem::isCategoryAdded(const sead::SafeString& rName) {
    return findResourceCategoryIter(rName) != mCategories.end();
}

/**
 * Checks if a category holds no resources.
 * @param rName category name
 * @return whether the category is empty or missing
 */
bool ResourceSystem::isEmptyCategoryResource(const sead::SafeString& rName) {
    auto iter = findResourceCategoryIter(rName);
    if (iter == mCategories.end()) {
        return true;
    }

    return (*iter)->mResources.isEmpty();
}

/**
 * Creates every resource listed for a category in the category table.
 * @param rName category name
 * @param pEvent event that cancels the creation when signaled, or null
 * @return whether all resources were created
 */
bool ResourceSystem::createCategoryResourceAll(const sead::SafeString& rName,
                                               sead::Event* pEvent) {
    if (!mResourceCategoryTable) {
        return false;
    }

    auto iter = findResourceCategoryIter(rName);
    if (iter == mCategories.end()) {
        return false;
    }

    for (s32 i = 0; i < mResourceCategoryTable->getSize(); i++) {
        ByamlIter categoryIter;
        if (!mResourceCategoryTable->tryGetIterByIndex(&categoryIter, i)) {
            continue;
        }

        const char* categoryName = nullptr;
        if (!categoryIter.tryGetStringByKey(&categoryName, "Category")) {
            continue;
        }

        if (!isEqualString(categoryName, rName.cstr())) {
            continue;
        }

        ByamlIter arcsIter;
        if (!categoryIter.tryGetIterByKey(&arcsIter, "Arcs")) {
            continue;
        }

        bool isLocalized = false;
        categoryIter.tryGetBoolByKey(&isLocalized, "Localized");
        for (s32 j = 0; j < arcsIter.getSize(); j++) {
            const char* arcName = nullptr;
            if (!arcsIter.tryGetStringByIndex(&arcName, j)) {
                continue;
            }

            StringTmp<128> localizedName;
            if (isLocalized) {
                makeLocalizedArchivePath(&localizedName, arcName);
                arcName = localizedName.cstr();
            }

            createResource(arcName, *iter, nullptr);
            if (pEvent && pEvent->wait(sead::TickSpan(0))) {
                return false;
            }
        }
    }

    return true;
}

/**
 * Removes a category and releases its resources.
 * @param rName category name
 */
void ResourceSystem::removeCategory(const sead::SafeString& rName) {
    ResourceAudioInfo audioInfo(mAudioPlayerA, mAudioPlayerB, "SoundData/");
    auto iter = findResourceCategoryIter(rName);
    if (iter == mCategories.end()) {
        return;
    }

    (*iter)->mResources.forEach(&cleanupResGraphicsFile);
    ResourceCategory* category = *iter;
    {
        ResourceAudio audio(&audioInfo);
        using MapImpl = sead::TreeMapImpl<sead::SafeString>;
        sead::Delegate1<ResourceAudio, MapImpl::Node*> delegate(
            &audio, &ResourceAudio::disableSoundMemoryPoolHandler);
        category->mResources.MapImpl::forEach(delegate);
    }

    (*iter)->mResources.clear();
    mCategories.remove(iter.getIndex());
}

/**
 * Finds a resource in all categories.
 * @param rPath resource path
 * @return resource, or null
 */
Resource* ResourceSystem::findResource(const sead::SafeString& rPath) {
    for (auto iter = mCategories.begin(); iter != mCategories.end(); ++iter) {
        auto* node = (*iter)->mResources.find(rPath);
        if (node) {
            return node->value();
        }
    }

    return nullptr;
}

/**
 * Finds a resource in all categories.
 * @param rPath resource path
 * @param pOutIter output category iterator
 * @return resource, or null
 */
Resource* ResourceSystem::findResourceCore(
    const sead::SafeString& rPath, sead::RingBuffer<ResourceCategory*>::iterator* pOutIter) {
    for (auto iter = mCategories.begin(); iter != mCategories.end(); ++iter) {
        auto* node = (*iter)->mResources.find(rPath);
        if (!node) {
            continue;
        }

        if (pOutIter) {
            *pOutIter = iter;
        }

        return node->value();
    }

    return nullptr;
}

/**
 * Finds a resource, creating it in the current category if needed.
 * @param rPath resource path
 * @param pExt archive extension
 * @return resource
 */
Resource* ResourceSystem::findOrCreateResource(const sead::SafeString& rPath, const char* pExt) {
    Resource* resource = findResource(rPath);
    if (resource) {
        return resource;
    }

    return createResource(rPath, findResourceCategory(rPath), pExt);
}

/**
 * Gets the category new resources are created in.
 * @param rPath resource path
 * @return category
 */
ResourceSystem::ResourceCategory* ResourceSystem::findResourceCategory(
    const sead::SafeString& rPath) {
    return *findResourceCategoryIter(mCurrentCategoryName ? mCurrentCategoryName : "Scene");
}

/**
 * Sets the category new resources are created in.
 * @param pName category name
 */
void ResourceSystem::setCurrentCategory(const char* pName) {
    mCurrentCategoryName = pName;
}

/**
 * Finds the category of an archive in the category table.
 * @param rPath archive path
 * @return category name, or null
 */
const char* ResourceSystem::findCategoryNameFromTable(const sead::SafeString& rPath) const {
    if (!mResourceCategoryTable) {
        return nullptr;
    }

    for (s32 i = 0; i < mResourceCategoryTable->getSize(); i++) {
        ByamlIter categoryIter;
        if (!mResourceCategoryTable->tryGetIterByIndex(&categoryIter, i)) {
            continue;
        }

        const char* categoryName = nullptr;
        if (!categoryIter.tryGetStringByKey(&categoryName, "Category")) {
            continue;
        }

        ByamlIter arcsIter;
        categoryIter.tryGetIterByKey(&arcsIter, "Arcs");
        bool isLocalized = false;
        categoryIter.tryGetBoolByKey(&isLocalized, "Localized");
        for (s32 j = 0; j < arcsIter.getSize(); j++) {
            const char* arcName = nullptr;
            if (!arcsIter.tryGetStringByIndex(&arcName, j)) {
                continue;
            }

            StringTmp<128> localizedName;
            if (isLocalized) {
                if (isEqualString(rPath, "TrialRating")) {
                    makeLocalizedArchivePathByCountryCode(&localizedName, rPath);
                } else {
                    makeLocalizedArchivePath(&localizedName, arcName);
                }

                arcName = localizedName.cstr();
            }

            if (isEqualString(arcName, rPath.cstr())) {
                return categoryName;
            }
        }
    }

    return nullptr;
}

/**
 * Gets the table entry of a category.
 * @param pIter output iterator
 * @param rName category name
 * @return whether the category is in the table
 */
bool ResourceSystem::tryGetTableCategoryIter(ByamlIter* pIter,
                                             const sead::SafeString& rName) const {
    if (!mResourceCategoryTable) {
        return false;
    }

    for (s32 i = 0; i < mResourceCategoryTable->getSize(); i++) {
        ByamlIter categoryIter;
        if (!mResourceCategoryTable->tryGetIterByIndex(&categoryIter, i)) {
            continue;
        }

        const char* categoryName = nullptr;
        if (!categoryIter.tryGetStringByKey(&categoryName, "Category")) {
            continue;
        }

        if (isEqualString(categoryName, rName.cstr()) &&
            mResourceCategoryTable->tryGetIterByIndex(pIter, i)) {
            return true;
        }
    }

    return false;
}

/**
 * Creates a resource in a category.
 * @param rPath resource path
 * @param pCategory category
 * @param pExt archive extension
 * @return resource, or null if the archive couldn't be loaded
 */
Resource* ResourceSystem::createResource(const sead::SafeString& rPath,
                                         ResourceCategory* pCategory, const char* pExt) {
    sead::ScopedCurrentHeapSetter setter(pCategory->mHeap);
    Resource* resource = nullptr;
    resource = pExt ? new Resource(rPath, loadArchiveWithExt(rPath, pExt)) : new Resource(rPath);
    pCategory->mResources.insert(rPath, resource);
    createResourceCore(this, resource, resource->getArchiveName(), nullptr);
    return resource->getFileArchive() ? resource : nullptr;
}

/**
 * Creates the graphics file and init data of a resource and its patch resources.
 * @param pSystem resource system
 * @param pResource resource
 * @param pArchiveName archive name
 * @param pParent resource patched by pResource, or null
 */
void createResourceCore(ResourceSystem* pSystem, Resource* pResource, const char* pArchiveName,
                        Resource* pParent) {
    if (!pParent) {
        pResource->loadPatchData();
    }

    StringTmp<256> fileName(pParent ? "%s_p.bfres" : "%s.bfres", pArchiveName);
    if (pResource->isExistFile(fileName)) {
        ByamlIter iter;
        nn::g3d::ResFile* resFile = nullptr;
        if (tryGetActorInitFileIter(&iter, pResource, "InitModel", nullptr)) {
            const char* textureArc = nullptr;
            iter.tryGetStringByKey(&textureArc, "TextureArc");
            if (textureArc) {
                resFile = pSystem
                              ->findOrCreateResource(StringTmp<256>("ObjectData/%s", textureArc),
                                                     nullptr)
                              ->getResFile();
            }
        }

        if (pParent && !resFile) {
            resFile = pParent->getResFile();
        }

        pResource->tryCreateResGraphicsFile(fileName, resFile);
        pResource->_B0 = reinterpret_cast<u64>(new ActorInitResourceData(pResource));
    }

    if (pResource->mPatchRes) {
        createResourceCore(pSystem, pResource->mPatchRes, pArchiveName, pResource);
    }
}

/**
 * Gets the graphics info of an archive.
 * @param pIter output iterator
 * @param rName archive name
 * @return whether the archive has graphics info
 */
bool ResourceSystem::tryGetGraphicsInfoIter(ByamlIter* pIter,
                                            const sead::SafeString& rName) const {
    if (!mResourceCategoryTable) {
        return false;
    }

    for (s32 i = 0; i < mResourceCategoryTable->getSize(); i++) {
        ByamlIter categoryIter;
        if (!mResourceCategoryTable->tryGetIterByIndex(&categoryIter, i)) {
            continue;
        }

        ByamlIter graphicsInfoIter;
        if (!categoryIter.tryGetIterByKey(&graphicsInfoIter, "GraphicsInfo")) {
            continue;
        }

        for (s32 j = 0; j < graphicsInfoIter.getSize(); j++) {
            graphicsInfoIter.tryGetIterByIndex(pIter, j);
            const char* arcName = nullptr;
            pIter->tryGetStringByKey(&arcName, "Arc");
            if (isEqualString(rName, arcName)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Gets the archive list of a category.
 * @param pIter output iterator
 * @param rCategory category name
 * @return whether the category has an archive list
 */
bool ResourceSystem::tryGetCategoryFileListIter(ByamlIter* pIter,
                                                const sead::SafeString& rCategory) const {
    for (s32 i = 0; i < mResourceCategoryTable->getSize(); i++) {
        ByamlIter categoryIter;
        if (!mResourceCategoryTable->tryGetIterByIndex(&categoryIter, i)) {
            continue;
        }

        const char* categoryName = nullptr;
        if (!categoryIter.tryGetStringByKey(&categoryName, "Category")) {
            continue;
        }

        if (!isEqualString(categoryName, rCategory.cstr())) {
            continue;
        }

        ByamlIter arcsIter;
        if (categoryIter.tryGetIterByKey(&arcsIter, "Arcs")) {
            *pIter = arcsIter;
            return true;
        }
    }

    return false;
}
}  // namespace al
