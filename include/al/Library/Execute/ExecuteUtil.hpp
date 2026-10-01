#pragma once

#include <container/seadPtrArray.h>

namespace al {
class ExecuteDirector;
class FunctorBase;
class IUseExecutor;
class LayoutActor;
class LiveActor;
class ModelDrawerBase;

using ModelDrawerArray = sead::PtrArray<ModelDrawerBase>;

void registerExecutorActorUpdate(LiveActor* pActor, ExecuteDirector* pDirector,
                                 const char* pListName);
void registerExecutorActorDraw(LiveActor* pActor, ExecuteDirector* pDirector,
                               const char* pListName);
void registerExecutorLayoutUpdate(LayoutActor* pLayout, ExecuteDirector* pDirector,
                                  const char* pListName);
void registerExecutorLayoutDraw(LayoutActor* pLayout, ExecuteDirector* pDirector,
                                const char* pListName);
void registerExecutorUser(IUseExecutor* pUser, ExecuteDirector* pDirector, const char* pListName);
void registerExecutorFunctor(const char* pListName, ExecuteDirector* pDirector,
                             const FunctorBase& rFunctor);
void registerExecutorFunctorDraw(const char* pListName, ExecuteDirector* pDirector,
                                 const FunctorBase& rFunctor);
}  // namespace al

namespace alActorSystemFunction {
void addToExecutorDraw(al::LiveActor* pActor);
void removeFromExecutorDraw(al::LiveActor* pActor);
al::ModelDrawerBase* tryCompletelyRemoveFromExecutorDraw(al::LiveActor* pActor);
void tryCompletelyRemoveFromExecutorDraw(al::LiveActor* pActor,
                                         al::ModelDrawerArray* pDrawers);
void addBackToExecutorDraw(al::LiveActor* pActor, al::ModelDrawerBase* pDrawer);
void addBackToExecutorDraw(al::LiveActor* pActor, al::ModelDrawerArray* pDrawers);
void removeFromExecutorDraw(al::LiveActor* pActor, al::ModelDrawerBase* pDrawer);
void removeFromExecutorDraw(al::LiveActor* pActor, al::ModelDrawerArray* pDrawers);
}  // namespace alActorSystemFunction

namespace alExecuteFunction {
void updateEffect(const al::ExecuteDirector* pDirector);
void updateEffectSystem(const al::ExecuteDirector* pDirector);
void updateEffectSystemStall(const al::ExecuteDirector* pDirector);
void updateEffectPlayer(const al::ExecuteDirector* pDirector);
void updateEffectHitStop(const al::ExecuteDirector* pDirector);
void updateEffectDemo(const al::ExecuteDirector* pDirector);
void updateEffectLayout(const al::ExecuteDirector* pDirector);
}  // namespace alExecuteFunction
