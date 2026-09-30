#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
class Event;
}  // namespace sead

namespace al {
class Resource;
class ByamlIter;

void addResourceCategory(const sead::SafeString& rName, s32 size, sead::Heap* pHeap);
bool isCategoryAdded(const sead::SafeString& rName);
bool isEmptyCategoryResource(const sead::SafeString& rName);
void createCategoryResourceAll(const sead::SafeString& rName, sead::Event* pEvent);
void removeResourceCategory(const sead::SafeString& rName);
const char* getResourceName(const Resource* pResource);
bool isExistResGraphicsFile(const Resource* pResource);
Resource* findResource(const sead::SafeString& rPath);
Resource* findOrCreateResource(const sead::SafeString& rPath, const char* pExt);
Resource* findOrCreateResourceCategory(const sead::SafeString& rPath,
                                       const sead::SafeString& rCategory, const char* pExt);
Resource* findOrCreateResourceSystemData(const char* pName, const char* pExt);
bool isExistResourceYaml(const Resource* pResource, const char* pName, const char* pSuffix);
const u8* findResourceYaml(const Resource* pResource, const char* pName, const char* pSuffix);
const void* tryFindStageParameterFileDesign(const sead::SafeString& rStageName,
                                            const sead::SafeString& rFileName, s32 scenarioNo);
bool tryGetCategoryFileListIter(ByamlIter* pIter, const sead::SafeString& rCategory);
void setCurrentCategoryName(const char* pName);
const u8* getBymlFromObjectResource(const sead::SafeString& rObjectName,
                                    const sead::SafeString& rBymlName);
const u8* tryGetBymlFromObjectResource(const sead::SafeString& rObjectName,
                                       const sead::SafeString& rBymlName);
const u8* getBymlFromLayoutResource(const sead::SafeString& rLayoutName,
                                    const sead::SafeString& rBymlName);
const u8* tryGetBymlFromLayoutResource(const sead::SafeString& rLayoutName,
                                       const sead::SafeString& rBymlName);
const u8* tryGetBymlFromArcName(const sead::SafeString& rArchiveName,
                                const sead::SafeString& rBymlName);
const u8* tryGetByml(const Resource* pResource, const sead::SafeString& rBymlName);
}  // namespace al
