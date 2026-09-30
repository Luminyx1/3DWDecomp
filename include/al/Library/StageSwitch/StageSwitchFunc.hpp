#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class FunctorBase;
class IUseStageSwitch;
class StageSwitchDirector;
struct PlacementInfo;

void initStageSwitch(IUseStageSwitch* pUser, const ActorInitInfo& rInfo);
void initStageSwitch(IUseStageSwitch* pUser, StageSwitchDirector* pDirector,
                     const PlacementInfo& rInfo);
bool tryInitStageSwitch(IUseStageSwitch* pUser, StageSwitchDirector* pDirector,
                        const PlacementInfo& rInfo);
bool isValidStageSwitch(const IUseStageSwitch* pUser, const char* pLinkName);
bool isOnStageSwitch(const IUseStageSwitch* pUser, const char* pLinkName);
void onStageSwitch(IUseStageSwitch* pUser, const char* pLinkName);
void offStageSwitch(IUseStageSwitch* pUser, const char* pLinkName);
bool tryOnStageSwitch(IUseStageSwitch* pUser, const char* pLinkName);
bool tryOffStageSwitch(IUseStageSwitch* pUser, const char* pLinkName);
bool tryOnStageSwitchInstant(IUseStageSwitch* pUser, const char* pLinkName);
bool tryOffStageSwitchInstant(IUseStageSwitch* pUser, const char* pLinkName);
bool isSameStageSwitch(const IUseStageSwitch* pUser, const IUseStageSwitch* pOther,
                       const char* pLinkName);
s32 findSwitchNo(const IUseStageSwitch* pUser, const char* pLinkName);
bool isUsingSwitchNo(const IUseStageSwitch* pUser, s32 switchNo);
bool isValidSwitchAppear(const IUseStageSwitch* pUser);
bool isOnSwitchAppear(const IUseStageSwitch* pUser);
bool isValidSwitchKill(const IUseStageSwitch* pUser);
bool isValidSwitchDeadOn(const IUseStageSwitch* pUser);
void onSwitchDeadOn(IUseStageSwitch* pUser);
void offSwitchDeadOn(IUseStageSwitch* pUser);
bool tryOnSwitchDeadOn(IUseStageSwitch* pUser);
bool tryOffSwitchDeadOn(IUseStageSwitch* pUser);
bool isValidSwitchStart(const IUseStageSwitch* pUser);
bool isOnSwitchStart(const IUseStageSwitch* pUser);
bool listenStageSwitchOn(IUseStageSwitch* pUser, const char* pLinkName,
                         const FunctorBase& rFunctor);
bool listenStageSwitchOff(IUseStageSwitch* pUser, const char* pLinkName,
                          const FunctorBase& rFunctor);
bool listenStageSwitchOnOff(IUseStageSwitch* pUser, const char* pLinkName,
                            const FunctorBase& rOnFunctor, const FunctorBase& rOffFunctor);
bool listenStageSwitchOnAppear(IUseStageSwitch* pUser, const FunctorBase& rFunctor);
bool listenStageSwitchOnOffAppear(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                  const FunctorBase& rOffFunctor);
bool listenStageSwitchOnKill(IUseStageSwitch* pUser, const FunctorBase& rFunctor);
bool listenStageSwitchOnOffKill(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                const FunctorBase& rOffFunctor);
bool listenStageSwitchOnStart(IUseStageSwitch* pUser, const FunctorBase& rFunctor);
bool listenStageSwitchOnOffStart(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                 const FunctorBase& rOffFunctor);
bool listenStageSwitchOnStop(IUseStageSwitch* pUser, const FunctorBase& rFunctor);
bool listenStageSwitchOnGoalItemGet(IUseStageSwitch* pUser, const FunctorBase& rFunctor);
}  // namespace al
