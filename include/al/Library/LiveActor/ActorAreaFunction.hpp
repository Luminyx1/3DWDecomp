#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjGroup;
class IUseAreaObj;
class LiveActor;
class SwitchKeepOnAreaGroup;

AreaObj* tryGetAreaObjPlayerAll(const LiveActor* pActor, const AreaObjGroup* pGroup);
bool isInAreaObjPlayerAll(const LiveActor* pActor, const AreaObj* pArea);
bool isInAreaObjPlayerAll(const LiveActor* pActor, const AreaObjGroup* pGroup);
bool isInAreaObjPlayerAnyOne(const LiveActor* pActor, const AreaObj* pArea);
bool isInAreaObjPlayerAnyOne(const LiveActor* pActor, const AreaObjGroup* pGroup);
bool tryIsInAreaPos(const AreaObj* pArea, const sead::Vector3f& rPos);
AreaObj* createAreaObj(const ActorInitInfo& rInfo, const char* pName);
AreaObjGroup* createLinkAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo,
                                  const char* pLinkName, const char* pGroupName,
                                  const char* pAreaName);
bool isInDeathArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool isInDeathArea(const LiveActor* pActor);
bool isInWaterArea(const LiveActor* pActor);
bool isInWaterArea(const LiveActor* pActor, f32 offset);
bool isInWaterAreaNoSink(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
bool isInWaterAreaNoSink(const LiveActor* pActor);
bool isInPlayerControlOffArea(const LiveActor* pActor);
void registerAreaHostMtx(const IUseAreaObj* pAreaUser, const sead::Matrix34f* pMtx,
                         const ActorInitInfo& rInfo);
void registerAreaHostMtx(const LiveActor* pActor, const ActorInitInfo& rInfo);
void registerAreaSyncHostMtx(const IUseAreaObj* pAreaUser, const sead::Matrix34f* pMtx,
                             const ActorInitInfo& rInfo);
void registerAreaSyncHostMtx(const LiveActor* pActor, const ActorInitInfo& rInfo);
bool tryReviseVelocityInsideAreaObj(sead::Vector3f* pVelocity, LiveActor* pActor,
                                    AreaObjGroup* pGroup, const AreaObj* pArea);
SwitchKeepOnAreaGroup* tryCreateSwitchKeepOnAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo);
}  // namespace al
