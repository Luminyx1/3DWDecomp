#pragma once

#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class ActorItemInfo;
class HitSensor;
class LiveActor;

ActorItemInfo* addItem(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pItemName,
                       const char* pTiming, const char* pFactor, bool unk);
ActorItemInfo* addItem(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pItemName,
                       bool unk);
void setAppearItemFactor(const LiveActor* pActor, const char* pFactor, const HitSensor* pSensor);
void setAppearItemOffset(const LiveActor* pActor, const sead::Vector3f& rOffset);
void setAppearItemAttackerSensor(const LiveActor* pActor, const HitSensor* pSensor);
void appearItem(const LiveActor* pActor);
void appearItem(const LiveActor* pActor, const sead::Vector3f& rTrans, const sead::Vector3f& rFront,
                const HitSensor* pSensor);
void appearItem(const LiveActor* pActor, const sead::Vector3f& rTrans, const sead::Vector3f& rFront);
void appearItemTiming(const LiveActor* pActor, const char* pTiming);
void appearItemTiming(const LiveActor* pActor, const char* pTiming, const sead::Vector3f& rTrans,
                      const sead::Vector3f& rFront, const HitSensor* pSensor, bool unk);
void appearItemTiming(const LiveActor* pActor, const char* pTiming, const sead::Vector3f& rTrans,
                      const sead::Vector3f& rFront);
void acquirerItem(const LiveActor* pActor, HitSensor* pSensor, const char* pItemName);
}  // namespace al
