#include "Library/Memory/HeapUtil.hpp"

#include <heap/seadHeapMgr.h>

#include "Library/File/FileUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/System/SystemKit.hpp"
#include "Project/Memory/MemorySystem.hpp"

namespace alAudioHeapFunction {
void createAudioResourceHeapLayer(al::AudioResourceDirector* pDirector,
                                  const sead::SafeString& rName);
void tryCreateAudioResourceHeapLayer(al::AudioResourceDirector* pDirector,
                                     const sead::SafeString& rName);
void destroyAudioResourceHeapLayer(al::AudioResourceDirector* pDirector,
                                   const sead::SafeString& rName);
}  // namespace alAudioHeapFunction

namespace al {
inline MemorySystem* getMemorySystem() {
    return alProjectInterface::getSystemKit()->getMemorySystem();
}

/**
 * Gets the stationed heap.
 * @return The heap.
 */
sead::Heap* getStationedHeap() {
    return getMemorySystem()->getStationedHeap();
}

/**
 * Gets the sequence heap.
 * @return The heap.
 */
sead::Heap* getSequenceHeap() {
    return getMemorySystem()->getSequenceHeap();
}

/**
 * Gets the scene resource heap.
 * @return The heap.
 */
sead::Heap* getSceneResourceHeap() {
    return getMemorySystem()->getSceneResourceHeap();
}

/**
 * Gets the scene heap.
 * @return The heap.
 */
sead::Heap* getSceneHeap() {
    return getMemorySystem()->getSceneHeap();
}

/**
 * Gets the course select resource heap.
 * @return The heap.
 */
sead::Heap* getCourseSelectResourceHeap() {
    return getMemorySystem()->getCourseSelectResourceHeap();
}

/**
 * Gets the course select heap.
 * @return The heap.
 */
sead::Heap* getCourseSelectHeap() {
    return getMemorySystem()->getCourseSelectHeap();
}

/**
 * Finds a named heap.
 * @param pHeapName Heap name.
 * @return The heap or nullptr.
 */
sead::Heap* tryFindNamedHeap(const char* pHeapName) {
    return getMemorySystem()->tryFindNamedHeap(pHeapName);
}

/**
 * Finds a named heap.
 * @param pHeapName Heap name.
 * @return The heap.
 */
sead::Heap* findNamedHeap(const char* pHeapName) {
    return getMemorySystem()->findNamedHeap(pHeapName);
}

/**
 * Registers a named heap.
 * @param pHeap The heap.
 * @param pHeapName Heap name.
 */
void addNamedHeap(sead::Heap* pHeap, const char* pHeapName) {
    getMemorySystem()->addNamedHeap(pHeap, pHeapName);
}

/**
 * Unregisters a named heap.
 * @param pHeapName Heap name.
 */
void removeNamedHeap(const char* pHeapName) {
    getMemorySystem()->removeNamedHeap(pHeapName);
}

/**
 * Creates the sequence heap and its resource category.
 */
void createSequenceHeap() {
    getMemorySystem()->createSequenceHeap();
    addResourceCategory("Sequence", 0x10, getMemorySystem()->getSequenceHeap());
    setCurrentCategoryName("Sequence");
    clearFileLoaderEntry();
    if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
        alAudioHeapFunction::createAudioResourceHeapLayer(director, "Sequence");
    }
}

/**
 * Frees everything in the sequence heap and recreates its resource category.
 */
void freeAllSequenceHeap() {
    removeResourceCategory("Sequence");
    if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
        alAudioHeapFunction::destroyAudioResourceHeapLayer(director, "Sequence");
    }
    getMemorySystem()->freeAllSequenceHeap();
    addResourceCategory("Sequence", 0x10, getMemorySystem()->getSequenceHeap());
    setCurrentCategoryName("Sequence");
    clearFileLoaderEntry();
    if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
        alAudioHeapFunction::createAudioResourceHeapLayer(director, "Sequence");
    }
}

/**
 * Sets the stage size adjuster of the memory system.
 * @param pAdjuster The adjuster.
 */
void setStageSizeAdjuster(StageSizeAdjuster* pAdjuster) {
    getMemorySystem()->setStageSizeAdjuster(pAdjuster);
}

/**
 * Sets the custom scene heap allocator of the memory system.
 * @param pAlloc The allocator.
 */
void setCustomSceneHeapAlloc(MemorySceneHeapCustomAlloc* pAlloc) {
    getMemorySystem()->setCustomSceneHeapAlloc(pAlloc);
}

/**
 * Forces the scene resource heap to be destroyed with the scene heap.
 */
void setForceSceneHeapResourceDestroy() {
    getMemorySystem()->setForceSceneHeapResourceDestroy();
}

/**
 * Creates the scene heap for a stage.
 * @param pStageName Stage name, or nullptr to also reset the custom allocator.
 */
void createSceneHeap(const char* pStageName) {
    if (!pStageName) {
        getMemorySystem()->setCustomSceneHeapAlloc(nullptr);
    }
    sead::ScopedCurrentHeapSetter setter(getSequenceHeap());
    if (getMemorySystem()->createSceneHeap(pStageName)) {
        addResourceCategory("Scene", 0x400, getMemorySystem()->getSceneResourceHeap());
        setCurrentCategoryName("Scene");
        clearFileLoaderEntry();
    }
    if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
        alAudioHeapFunction::tryCreateAudioResourceHeapLayer(director, "シーン");
    }
}

/**
 * Creates the scene resource heap for a stage.
 * @param pStageName Stage name.
 */
void createSceneResourceHeap(const char* pStageName) {
    sead::ScopedCurrentHeapSetter setter(getSequenceHeap());
    getMemorySystem()->createSceneResourceHeap(pStageName);
    addResourceCategory("Scene", 0x400, getMemorySystem()->getSceneResourceHeap());
    setCurrentCategoryName("Scene");
    clearFileLoaderEntry();
    if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
        alAudioHeapFunction::createAudioResourceHeapLayer(director, "Scene");
    }
}

/**
 * Checks whether the scene resource heap exists.
 * @return True if it exists.
 */
bool isCreatedSceneResourceHeap() {
    return getSceneResourceHeap() != nullptr;
}

/**
 * Destroys the scene heap and, if really freed, the scene resources.
 * @param removeCategory Whether to also destroy the scene resources.
 */
void destroySceneHeap(bool removeCategory) {
    if (getMemorySystem()->isReallyFreeSceneResource(removeCategory)) {
        removeResourceCategory("Scene");
        if (AudioResourceDirector* director = getMemorySystem()->getAudioResourceDirector()) {
            alAudioHeapFunction::destroyAudioResourceHeapLayer(director, "Scene");
        }
    }
    getMemorySystem()->destroySceneHeap(removeCategory);
}

/**
 * Creates the course select heaps and their resource category.
 */
void createCourseSelectHeap() {
    sead::ScopedCurrentHeapSetter setter(getSequenceHeap());
    getMemorySystem()->createCourseSelectHeap();
    addResourceCategory("CourseSelectResource", 0x80,
                        getMemorySystem()->getCourseSelectResourceHeap());
    setCurrentCategoryName("CourseSelectResource");
    clearFileLoaderEntry();
}

/**
 * Destroys the course select heaps and their resource category.
 */
void destroyCourseSelectHeap() {
    removeResourceCategory("CourseSelectResource");
    getMemorySystem()->destroyCourseSelectHeap();
}

/**
 * Sets the audio resource director of the memory system.
 * @param pDirector The director.
 */
void setAudioResourceDirectorToMemorySystem(AudioResourceDirector* pDirector) {
    getMemorySystem()->setAudioResourceDirector(pDirector);
}
}  // namespace al
