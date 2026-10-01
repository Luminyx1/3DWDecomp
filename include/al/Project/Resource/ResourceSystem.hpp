#pragma once

#include <basis/seadTypes.h>
#include <container/seadRingBuffer.h>
#include <container/seadStrTreeMap.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
class Event;
}  // namespace sead

namespace al {
class Resource;
class ByamlIter;
class SeadAudioPlayer;

class ResourceSystem {
public:
    struct ResourceCategory {
        ResourceCategory(const sead::SafeString& rName, sead::Heap* pHeap) {
            mName = rName;
            mHeap = pHeap;
        }

        sead::FixedSafeString<0x80> mName;
        sead::Heap* mHeap;
        sead::StrTreeMap<156, Resource*> mResources;
    };

    static_assert(sizeof(ResourceCategory) == 0xc0);

    using CategoryIterator = sead::RingBuffer<ResourceCategory*>::iterator;

    struct ResourceAudioInfo {
        ResourceAudioInfo(SeadAudioPlayer* pPlayerA, SeadAudioPlayer* pPlayerB,
                          const char* pPath)
            : mAudioPlayerA(pPlayerA), mAudioPlayerB(pPlayerB) {
            mFilePath.format(pPath);
        }

        SeadAudioPlayer* mAudioPlayerA;
        SeadAudioPlayer* mAudioPlayerB;
        sead::FixedSafeString<0x40> mFilePath;
    };

    ResourceSystem(const char* pArchivePath);

    ResourceCategory* addCategory(const sead::SafeString& rName, s32 size, sead::Heap* pHeap);
    Resource* findOrCreateResourceCategory(const sead::SafeString& rPath,
                                           const sead::SafeString& rCategory, const char* pExt);
    CategoryIterator findResourceCategoryIter(const sead::SafeString& rName);
    bool isCategoryAdded(const sead::SafeString& rName);
    bool isEmptyCategoryResource(const sead::SafeString& rName);
    bool createCategoryResourceAll(const sead::SafeString& rName, sead::Event* pEvent);
    Resource* createResource(const sead::SafeString& rPath, ResourceCategory* pCategory,
                             const char* pExt);
    void removeCategory(const sead::SafeString& rName);
    Resource* findResource(const sead::SafeString& rPath);
    Resource* findResourceCore(const sead::SafeString& rPath,
                               CategoryIterator* pOutIter);
    Resource* findOrCreateResource(const sead::SafeString& rPath, const char* pExt);
    ResourceCategory* findResourceCategory(const sead::SafeString& rPath);
    void setCurrentCategory(const char* pName);
    const char* findCategoryNameFromTable(const sead::SafeString& rPath) const;
    bool tryGetTableCategoryIter(ByamlIter* pIter, const sead::SafeString& rName) const;
    bool tryGetGraphicsInfoIter(ByamlIter* pIter, const sead::SafeString& rName) const;
    bool tryGetCategoryFileListIter(ByamlIter* pIter, const sead::SafeString& rCategory) const;

    void setAudioPlayer(SeadAudioPlayer* pPlayerA, SeadAudioPlayer* pPlayerB) {
        mAudioPlayerA = pPlayerA;
        mAudioPlayerB = pPlayerB;
    }

    sead::FixedRingBuffer<ResourceCategory*, 18> mCategories;
    ByamlIter* mResourceCategoryTable = nullptr;
    const char* mCurrentCategoryName = nullptr;
    SeadAudioPlayer* mAudioPlayerA = nullptr;
    SeadAudioPlayer* mAudioPlayerB = nullptr;
};

static_assert(sizeof(ResourceSystem) == 0xc8);
}  // namespace al
