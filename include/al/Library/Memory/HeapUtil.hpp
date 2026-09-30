#pragma once

#include <heap/seadHeap.h>

namespace al {
class AudioResourceDirector;
class StageSizeAdjuster;
class MemorySceneHeapCustomAlloc;

sead::Heap* getStationedHeap();
sead::Heap* getSequenceHeap();
sead::Heap* getSceneResourceHeap();
sead::Heap* getSceneHeap();
sead::Heap* getCourseSelectResourceHeap();
sead::Heap* getCourseSelectHeap();
sead::Heap* tryFindNamedHeap(const char* pHeapName);
sead::Heap* findNamedHeap(const char* pHeapName);
void addNamedHeap(sead::Heap* pHeap, const char* pHeapName);
void removeNamedHeap(const char* pHeapName);
void createSequenceHeap();
void freeAllSequenceHeap();
void setStageSizeAdjuster(StageSizeAdjuster* pAdjuster);
void setCustomSceneHeapAlloc(MemorySceneHeapCustomAlloc* pAlloc);
void setForceSceneHeapResourceDestroy();
void createSceneHeap(const char* pStageName);
void createSceneResourceHeap(const char* pStageName);
bool isCreatedSceneResourceHeap();
void destroySceneHeap(bool removeCategory);
void createCourseSelectHeap();
void destroyCourseSelectHeap();
void setAudioResourceDirectorToMemorySystem(AudioResourceDirector* pDirector);
}  // namespace al
