#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class JointSpringController;
class LiveActor;
class SkyProjection;
}  // namespace al

class DarkBowser;

/** @brief Shared helpers for Fury Bowser (DarkBowser) and its battle states. */
namespace DarkBowserUtil {

/** @brief A chain of joint controllers (an arm or the tail) that can be aimed as a whole. */
struct ControlledJointChain {
    void* _0[10];
    s32 _50 = 5;
    s32 _54 = 0;
};
static_assert(sizeof(ControlledJointChain) == 0x58);

void initDarkBowserJointControllers(al::LiveActor* pActor, const al::ActorInitInfo& rInfo,
                                    const char* pSuffix, ControlledJointChain& rChainA,
                                    ControlledJointChain& rChainB,
                                    sead::PtrArray<al::JointSpringController>* pSprings);
void jointChainRelease(ControlledJointChain& rChain);
void jointChaimAimNoInterpolate(ControlledJointChain& rChain, const sead::Vector3f& rTarget);
void jointChainSetPowerRate(ControlledJointChain& rChain, f32 rate);
void setSkyToNight(al::LiveActor* pActor, al::SkyProjection* pSkyDay,
                   al::SkyProjection* pSkyNight, bool isInstant);
void setSkyToDay(al::LiveActor* pActor, al::SkyProjection* pSkyDay, al::SkyProjection* pSkyNight,
                 bool isInstant);
void hideDarkBowser(DarkBowser* pDarkBowser);
sead::Vector3f calculateArc(sead::Vector3f start, sead::Vector3f end, f32 height, f32 rate,
                            s32 arg4, s32 arg5);

}  // namespace DarkBowserUtil
