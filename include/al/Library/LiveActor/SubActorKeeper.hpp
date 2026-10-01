#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class LiveActor;

struct SubActorInfo {
    SubActorInfo();

    SubActorInfo(LiveActor* pActor, u32 syncType) : mSubActor(pActor), mSyncType(syncType) {}

    LiveActor* mSubActor = nullptr;
    void* _8 = nullptr;
    u32 mSyncType = 0;
};

class SubActorKeeper {
public:
    SubActorKeeper(LiveActor* pRootActor, const ActorInitInfo& rInfo, const char* pSuffix,
                   s32 maxSubActors);

    static SubActorKeeper* tryCreate(LiveActor* pRootActor, const ActorInitInfo& rInfo,
                                     const char* pSuffix, s32 maxSubActors);
    static SubActorKeeper* createNoFile(LiveActor* pRootActor, const ActorInitInfo& rInfo,
                                        s32 maxSubActors);
    void registerSubActor(LiveActor* pSubActor, u32 syncType);

    s32 getSubActorNum() const { return mCount; }
    SubActorInfo* getSubActorInfo(s32 index) const { return mInfos[index]; }

    LiveActor* mRootActor;
    s32 mMaxCount = 0;
    s32 mCount = 0;
    SubActorInfo** mInfos = nullptr;
};
}  // namespace al
