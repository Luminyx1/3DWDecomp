#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class ClippingJudge;
class LiveActor;
class Resource;

class ActorAlphaCtrl {
public:
    struct SphereInfo {
        SphereInfo();

        void init(const ByamlIter& rIter, LiveActor* pActor);
        f32 update(LiveActor* pActor, const ClippingJudge* pJudge);

        f32 mNearDist = 200.0f;
        f32 mFarDist = 450.0f;
        const sead::Matrix34f* mJointMtx = nullptr;
        sead::Vector3f mPosOffset = sead::Vector3f::zero;
    };
    static_assert(sizeof(SphereInfo) == 0x20);

    static ActorAlphaCtrl* tryCreate(LiveActor* pActor, const Resource* pResource,
                                     const char* pFileName);

    ActorAlphaCtrl(const ByamlIter& rIter, LiveActor* pActor);

    f32 update(const ClippingJudge* pJudge);

    LiveActor* mActor;
    SphereInfo* mSphereInfos;
    s32 mSphereInfoNum;
    f32 mAlpha = 1.0f;
    SphereInfo mSphereInfo;
    bool mIsOn = true;
};
static_assert(sizeof(ActorAlphaCtrl) == 0x40);
}  // namespace al
