#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

namespace al {
class StageInfo;
struct PlacementInfo;

class StageResourceList {
public:
    StageResourceList(const char* pStageName, s32 scenarioNo, const char* pResourceType,
                      bool isOneResource);

    StageInfo* initZoneInfo(PlacementInfo& rZoneInfo, s32 scenarioNo, const char* pResourceType,
                            PlacementInfo* pParentInfo);
    void initZoneInfoRecursive(const char* pStageName, const char* pListName, s32 scenarioNo,
                               const char* pResourceType, const char* pChildListName,
                               PlacementInfo* pParentInfo);
    s32 getStageResourceNum() const;
    StageInfo* getStageInfo(s32 index) const;
    StageInfo* findStageInfo(const char* pName) const;

    bool isOneResource() const { return mIsOneResource; }

    sead::PtrArray<StageInfo> mStageInfos;
    bool mIsOneResource;
    s32 mLastZoneId = -1;
};

void makeStageDataArchivePath(sead::BufferedSafeString* pOut, const char* pStageName,
                              s32 scenarioNo, const char* pResourceType, bool isOneResource);
bool isOneStageDataArchiveExists(const char* pStageName);
}  // namespace al
