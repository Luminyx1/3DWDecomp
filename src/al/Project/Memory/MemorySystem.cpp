#include "Project/Memory/MemorySystem.hpp"

#include <basis/seadRawPrint.h>
#include <filedevice/seadFileDeviceMgr.h>

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace sead::system {
void Halt();
}

namespace al {
/**
 * Creates the stationed heap and registers the allocation failure callback.
 * @param pHeap parent heap
 * @param a unused
 * @param b unused
 * @param c unused
 */
MemorySystem::MemorySystem(sead::Heap* pHeap, u64 a, u64 b, u64 c)
    : mAllocFailedCallback(this, &MemorySystem::allocFailedCallbackFunc) {
    sead::HeapMgr::instance()->setAllocFailedCallback(&mAllocFailedCallback);
    mStationedHeap = sead::ExpHeap::create(pHeap->getMaxAllocatableSize(8) - 0x64000,
                                           "StationedHeap", pHeap, 8,
                                           sead::Heap::cHeapDirection_Forward, false);
    sead::ScopedCurrentHeapSetter setter(mStationedHeap);
    mHeapList.allocBuffer(32, nullptr);
    mIsExistFileResource = sead::FileDeviceMgr::instance()->getMainFileDevice()->isExistFile(
        StringTmp<64>("%s.szs", "SystemData/MemorySystem"));
}

/**
 * Halts when an allocation fails, except for the play reporter heap.
 * @param pArg allocation failure info
 */
void MemorySystem::allocFailedCallbackFunc(const sead::HeapMgr::AllocFailedCallbackArg* pArg) {
    if (isEqualString(pArg->heap->getName(), "PlayReporter")) {
        return;
    }
    sead::system::Halt();
}

/**
 * Creates the sequence heap.
 */
void MemorySystem::createSequenceHeap() {
    mSequenceHeap = sead::ExpHeap::create(0, "SequenceHeap", nullptr, 8,
                                          sead::Heap::cHeapDirection_Forward, false);
}

/**
 * Creates the course select stationed heap and its resource category.
 */
void MemorySystem::createCourseSelectStationedHeap() {
    mCourseSelectStationedHeap =
        sead::ExpHeap::create(0xc800000, "CourseSelectStationed", mSequenceHeap, 8,
                              sead::Heap::cHeapDirection_Forward, false);
    addResourceCategory("Stationed[CourseSelect]", 0x100, mCourseSelectStationedHeap);
}

/**
 * Destroys the course select stationed heap and its resource category.
 */
void MemorySystem::destroyCourseSelectStationedHeap() {
    removeResourceCategory("Stationed[CourseSelect]");
    mCourseSelectStationedHeap->freeAll();
    mCourseSelectStationedHeap->destroy();
    mCourseSelectStationedHeap = nullptr;
}

/**
 * Frees everything in the sequence heap.
 */
void MemorySystem::freeAllSequenceHeap() {
    mSequenceHeap->freeAll();
}

/**
 * Creates the scene heap, creating the scene resource heap first if needed.
 * @param pStageName stage name
 * @return whether the scene resource heap was created
 */
bool MemorySystem::createSceneHeap(const char* pStageName) {
    sead::Heap* resourceHeap = mSceneResourceHeap;
    if (!resourceHeap) {
        createSceneResourceHeap(pStageName);
    }
    if (mCustomAlloc) {
        mCustomAlloc->createSceneHeap(resourceHeap == nullptr);
    }
    mSceneHeap = sead::FrameHeap::create(0, "SceneHeap", nullptr, 8,
                                         sead::Heap::cHeapDirection_Forward, false);
    mSceneHeap->enableWarning(false);
    return resourceHeap == nullptr;
}

namespace {
inline u64 findSceneResourceHeapSize(const char* pStageName, u64 defaultSize) {
    ByamlIter heapSizeIter(
        findOrCreateResource("SystemData/MemorySystem", nullptr)->getByml("HeapSizeDefine"));
    for (s32 i = 0; i < heapSizeIter.getSize(); i++) {
        ByamlIter entryIter;
        heapSizeIter.tryGetIterByIndex(&entryIter, i);
        const char* stageName = nullptr;
        entryIter.tryGetStringByKey(&stageName, "Stage");
        if (isEqualString(stageName, pStageName)) {
            f32 sizeMB = 0.0f;
            entryIter.tryGetFloatByKey(&sizeMB, "SceneResource");
            return sizeMB * 1024.0f * 1024.0f;
        }
    }
    return defaultSize;
}
}  // namespace

/**
 * Creates the scene resource heap with the size defined for the stage.
 * @param pStageName stage name
 */
void MemorySystem::createSceneResourceHeap(const char* pStageName) {
    u64 size = 0x4600000;
    bool isDefaultSize = true;
    if (pStageName && mIsExistFileResource) {
        size = findSceneResourceHeapSize(pStageName, size);
        if (mStageSizeAdjuster) {
            s64 adjustedSize = mStageSizeAdjuster->adjustSceneResourceSize(pStageName, size,
                                                                           mCustomAlloc);
            isDefaultSize = size == adjustedSize;
            size = adjustedSize;
        }
    }
    if (isDefaultSize && size < 0x7800000) {
        size = 0x7800000;
    }
    mSceneResourceHeap = sead::FrameHeap::create(size, "SceneHeapResource", nullptr, 8,
                                                 sead::Heap::cHeapDirection_Forward, true);
    mSceneResourceHeap->enableWarning(false);
}

/**
 * Destroys the scene heap and, if allowed, the scene resource heap.
 * @param isRemoveCategory whether the scene resource category is removed
 */
void MemorySystem::destroySceneHeap(bool isRemoveCategory) {
    mSceneHeap->destroy();
    mSceneHeap = nullptr;
    bool isFree = mCustomAlloc ? mCustomAlloc->isFreeSceneResource(isRemoveCategory) :
                                 isRemoveCategory;
    if (isFree) {
        mSceneResourceHeap->destroy();
        mSceneResourceHeap = nullptr;
    }
}

/**
 * Checks if the scene resources will really be freed.
 * @param isRemoveCategory whether the scene resource category is removed
 * @return whether the scene resources are freed
 */
bool MemorySystem::isReallyFreeSceneResource(bool isRemoveCategory) const {
    if (!isRemoveCategory) {
        return false;
    }
    if (!mCustomAlloc) {
        return true;
    }
    return mCustomAlloc->isReallyFreeSceneResource();
}

/**
 * Forces the scene resource heap to be destroyed with the scene heap.
 */
void MemorySystem::setForceSceneHeapResourceDestroy() {
    if (mCustomAlloc) {
        mCustomAlloc->setForceSceneHeapResourceDestroy();
    }
}

/**
 * Creates the course select heaps.
 */
void MemorySystem::createCourseSelectHeap() {
    mCourseSelectResourceHeap =
        sead::FrameHeap::create(0x5000000, "CourseSelectHeapResource", nullptr, 8,
                                sead::Heap::cHeapDirection_Reverse, true);
    mCourseSelectResourceHeap->enableWarning(false);
    mCourseSelectHeap = sead::FrameHeap::create(0xfc00000, "CourseSelectHeapScene", nullptr, 8,
                                                sead::Heap::cHeapDirection_Reverse, false);
    mCourseSelectHeap->enableWarning(false);
}

/**
 * Destroys the course select heaps.
 */
void MemorySystem::destroyCourseSelectHeap() {
    mCourseSelectHeap->destroy();
    mCourseSelectHeap = nullptr;
    mCourseSelectResourceHeap->destroy();
    mCourseSelectResourceHeap = nullptr;
}

/**
 * Frees everything in the player heap.
 */
void MemorySystem::freeAllPlayerHeap() {
    mPlayerHeap->freeAll();
}

/**
 * Finds a named heap.
 * @param pHeapName heap name
 * @return heap, or null
 */
sead::Heap* MemorySystem::tryFindNamedHeap(const char* pHeapName) const {
    auto* node = mHeapList.find(pHeapName);
    if (!node) {
        return nullptr;
    }
    return node->value();
}

/**
 * Finds a named heap.
 * @param pHeapName heap name
 * @return heap, or null
 */
sead::Heap* MemorySystem::findNamedHeap(const char* pHeapName) const {
    auto* node = mHeapList.find(pHeapName);
    if (!node) {
        return nullptr;
    }
    return node->value();
}

/**
 * Registers a named heap.
 * @param pHeap heap
 * @param pHeapName name, or null to use the heap's name
 */
void MemorySystem::addNamedHeap(sead::Heap* pHeap, const char* pHeapName) {
    mHeapList.insert(pHeapName ? pHeapName : pHeap->getName().cstr(), pHeap);
}

/**
 * Unregisters a named heap.
 * @param pHeapName heap name
 */
void MemorySystem::removeNamedHeap(const char* pHeapName) {
    sead::SafeString name = pHeapName;
    if (mHeapList.find(name)) {
        mHeapList.erase(name);
    }
}
}  // namespace al
