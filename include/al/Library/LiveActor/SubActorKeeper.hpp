#pragma once

#include <basis/seadTypes.h>
#include <prim/seadBitFlag.h>

namespace al {
    class LiveActor;
    class ActorInitInfo;

    /// Which of the parent's state changes a sub actor follows.
    namespace SubActorSync {
        constexpr u32 cAppear = 1 << 0;
        constexpr u32 cClipping = 1 << 1;
        constexpr u32 cHide = 1 << 2;
    };

    struct SubActorInfo {
        SubActorInfo();

        LiveActor* mSubActor = nullptr;     // _0
        void* _8 = nullptr;
        sead::BitFlag32 mSyncType;          // _10
    };

    /// Holds the sub actors (parts) of an actor, read from its InitSubActor resource file.
    class SubActorKeeper {
    public:
        static SubActorKeeper* tryCreate(LiveActor* pRootActor, const ActorInitInfo& rInfo, const char* pSuffix, s32 maxActors);
        static SubActorKeeper* createNoFile(LiveActor* pRootActor, const ActorInitInfo& rInfo, s32 maxActors);

        SubActorKeeper(LiveActor* pRootActor, const ActorInitInfo& rInfo, const char* pSuffix, s32 maxActors);

        void registerSubActor(LiveActor* pSubActor, u32 syncType);

        LiveActor* mRootActor;              // _0
        s32 mMaxActorCount;                 // _8
        s32 mCurActorCount;                 // _C
        SubActorInfo** mActorInfos;         // _10
    };
};
