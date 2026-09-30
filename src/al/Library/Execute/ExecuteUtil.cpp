#include "Library/Execute/ExecuteUtil.hpp"

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Execute/ActorSystemFunction.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/Execute/ExecuteRequestKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Library/Model/ModelKeeper.hpp"

namespace al {
/**
 * Registers an actor to an update list.
 * @param pActor The actor.
 * @param pDirector The execute director.
 * @param pListName List name.
 */
void registerExecutorActorUpdate(LiveActor* pActor, ExecuteDirector* pDirector,
                                 const char* pListName) {
    pDirector->registerActorUpdate(pActor, pListName);
}

/**
 * Registers an actor to a draw list.
 * @param pActor The actor.
 * @param pDirector The execute director.
 * @param pListName List name.
 */
void registerExecutorActorDraw(LiveActor* pActor, ExecuteDirector* pDirector,
                               const char* pListName) {
    pDirector->registerActorDraw(pActor, pListName);
}

/**
 * Registers a layout to an update list.
 * @param pLayout The layout.
 * @param pDirector The execute director.
 * @param pListName List name.
 */
void registerExecutorLayoutUpdate(LayoutActor* pLayout, ExecuteDirector* pDirector,
                                  const char* pListName) {
    pDirector->registerLayoutUpdate(pLayout, pListName);
}

/**
 * Registers a layout to a draw list.
 * @param pLayout The layout.
 * @param pDirector The execute director.
 * @param pListName List name.
 */
void registerExecutorLayoutDraw(LayoutActor* pLayout, ExecuteDirector* pDirector,
                                const char* pListName) {
    pDirector->registerLayoutDraw(pLayout, pListName);
}

/**
 * Registers an executor user.
 * @param pUser The user.
 * @param pDirector The execute director.
 * @param pListName List name.
 */
void registerExecutorUser(IUseExecutor* pUser, ExecuteDirector* pDirector, const char* pListName) {
    pDirector->registerUser(pUser, pListName);
}

/**
 * Registers a functor.
 * @param pListName List name.
 * @param pDirector The execute director.
 * @param rFunctor The functor.
 */
void registerExecutorFunctor(const char* pListName, ExecuteDirector* pDirector,
                             const FunctorBase& rFunctor) {
    pDirector->registerFunctor(rFunctor, pListName);
}

/**
 * Registers a functor to a draw list.
 * @param pListName List name.
 * @param pDirector The execute director.
 * @param rFunctor The functor.
 */
void registerExecutorFunctorDraw(const char* pListName, ExecuteDirector* pDirector,
                                 const FunctorBase& rFunctor) {
    pDirector->registerFunctorDraw(rFunctor, pListName);
}
}  // namespace al

namespace alActorSystemFunction {
/**
 * Requests adding an actor to its movement executors.
 * @param pActor The actor.
 */
void addToExecutorMovement(al::LiveActor* pActor) {
    pActor->mActorExecuteInfo->mRequestKeeper->request(
        pActor, al::ExecuteRequestKeeper::Request_Movement);
}

/**
 * Requests adding an actor to its drawers.
 * @param pActor The actor.
 */
void addToExecutorDraw(al::LiveActor* pActor) {
    pActor->mActorExecuteInfo->mRequestKeeper->request(pActor,
                                                       al::ExecuteRequestKeeper::Request_Draw);
}

/**
 * Requests removing an actor from its movement executors.
 * @param pActor The actor.
 */
void removeFromExecutorMovement(al::LiveActor* pActor) {
    pActor->mActorExecuteInfo->mRequestKeeper->request(
        pActor, al::ExecuteRequestKeeper::Request_RemoveFromMovement);
}

/**
 * Requests removing an actor from its drawers.
 * @param pActor The actor.
 */
void removeFromExecutorDraw(al::LiveActor* pActor) {
    pActor->mActorExecuteInfo->mRequestKeeper->request(
        pActor, al::ExecuteRequestKeeper::Request_RemoveFromDraw);
}

/**
 * Requests adding an actor to its drawers immediately after the update.
 * @param pActor The actor.
 */
void addToExecutorDrawImmediate(al::LiveActor* pActor) {
    pActor->mActorExecuteInfo->mRequestKeeper->request(
        pActor, al::ExecuteRequestKeeper::Request_DrawImmediate);
}

/**
 * Removes the actor's model from its first removeable drawer.
 * @param pActor The actor.
 * @return The removed drawer or nullptr.
 */
al::ModelDrawerBase* tryCompletelyRemoveFromExecutorDraw(al::LiveActor* pActor) {
    al::ModelDrawerBase* drawer = pActor->mActorExecuteInfo->removeOptDrawer();
    if (drawer) {
        drawer->removeModel(pActor->mModelKeeper->mModelCafe);
    }
    return drawer;
}

/**
 * Removes the actor's model from all removeable drawers.
 * @param pActor The actor.
 * @param pDrawers Receives the removed drawers.
 */
void tryCompletelyRemoveFromExecutorDraw(al::LiveActor* pActor,
                                         sead::PtrArray<al::ModelDrawerBase>* pDrawers) {
    pDrawers->allocBuffer(4, nullptr);
    while (true) {
        al::ModelDrawerBase* drawer = tryCompletelyRemoveFromExecutorDraw(pActor);
        if (!drawer) {
            break;
        }
        pDrawers->pushBack(drawer);
    }
}

/**
 * Adds a drawer back to the actor and the actor's model back to the drawer.
 * @param pActor The actor.
 * @param pDrawer The drawer.
 */
void addBackToExecutorDraw(al::LiveActor* pActor, al::ModelDrawerBase* pDrawer) {
    pActor->mActorExecuteInfo->addDrawer(pDrawer);
    pDrawer->addModel(pActor->mModelKeeper->mModelCafe);
}

/**
 * Adds drawers back to the actor and the actor's model back to them.
 * @param pActor The actor.
 * @param pDrawers The drawers.
 */
void addBackToExecutorDraw(al::LiveActor* pActor, sead::PtrArray<al::ModelDrawerBase>* pDrawers) {
    for (s32 i = 0; i < pDrawers->size(); i++) {
        al::ModelDrawerBase* drawer = pDrawers->at(i);
        pActor->mActorExecuteInfo->addDrawer(drawer);
        drawer->addModel(pActor->mModelKeeper->mModelCafe);
    }
}

/**
 * Removes the actor's model from a drawer and the drawer from the actor.
 * @param pActor The actor.
 * @param pDrawer The drawer.
 */
void removeFromExecutorDraw(al::LiveActor* pActor, al::ModelDrawerBase* pDrawer) {
    pDrawer->removeModel(pActor->mModelKeeper->mModelCafe);
    pActor->mActorExecuteInfo->removeDrawer(pDrawer);
}

/**
 * Removes the actor's model from drawers and the drawers from the actor.
 * @param pActor The actor.
 * @param pDrawers The drawers.
 */
void removeFromExecutorDraw(al::LiveActor* pActor, sead::PtrArray<al::ModelDrawerBase>* pDrawers) {
    for (s32 i = 0; i < pDrawers->size(); i++) {
        al::ModelDrawerBase* drawer = pDrawers->unsafeAt(i);
        drawer->removeModel(pActor->mModelKeeper->mModelCafe);
        pActor->mActorExecuteInfo->removeDrawer(drawer);
    }
}
}  // namespace alActorSystemFunction

namespace alExecuteFunction {
/**
 * Updates all effect groups.
 * @param pDirector The execute director.
 */
void updateEffect(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("エフェクト（３Ｄ）");
    pDirector->executeList("エフェクト（プレイヤー）");
    pDirector->executeList("Effect (HitStop)");
    pDirector->executeList("エフェクト（カメラデモ）");
    pDirector->executeList("エフェクト（ベース２Ｄ）");
    pDirector->executeList("エフェクト（２Ｄ）");
    pDirector->executeList("エフェクト（下画面ベース２Ｄ）");
    pDirector->executeList("エフェクト（下画面２Ｄ）");
    pDirector->executeList("エフェクト（後処理）");
}

/**
 * Runs the effect pre and post processing.
 * @param pDirector The execute director.
 */
void updateEffectSystem(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("エフェクト（後処理）");
}

/**
 * Runs the effect pre and post processing without handling execute requests.
 * @param pDirector The execute director.
 */
void updateEffectSystemStall(const al::ExecuteDirector* pDirector) {
    pDirector->executeListStall("エフェクト（前処理）");
    pDirector->executeListStall("エフェクト（後処理）");
}

/**
 * Updates the effect groups used while only the player moves.
 * @param pDirector The execute director.
 */
void updateEffectPlayer(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("エフェクト（プレイヤー）");
    pDirector->executeList("エフェクト（ベース２Ｄ）");
    pDirector->executeList("エフェクト（２Ｄ）");
    pDirector->executeList("エフェクト（下画面ベース２Ｄ）");
    pDirector->executeList("エフェクト（下画面２Ｄ）");
    pDirector->executeList("エフェクト（後処理）");
}

/**
 * Updates the effect groups used during hit stop.
 * @param pDirector The execute director.
 */
void updateEffectHitStop(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("Effect (HitStop)");
    pDirector->executeList("エフェクト（ベース２Ｄ）");
    pDirector->executeList("エフェクト（２Ｄ）");
    pDirector->executeList("エフェクト（下画面ベース２Ｄ）");
    pDirector->executeList("エフェクト（下画面２Ｄ）");
    pDirector->executeList("エフェクト（後処理）");
}

/**
 * Updates the effect groups used during camera demos.
 * @param pDirector The execute director.
 */
void updateEffectDemo(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("エフェクト（カメラデモ）");
    pDirector->executeList("エフェクト（ベース２Ｄ）");
    pDirector->executeList("エフェクト（２Ｄ）");
    pDirector->executeList("エフェクト（下画面ベース２Ｄ）");
    pDirector->executeList("エフェクト（下画面２Ｄ）");
    pDirector->executeList("エフェクト（後処理）");
}

/**
 * Updates the 2D effect groups.
 * @param pDirector The execute director.
 */
void updateEffectLayout(const al::ExecuteDirector* pDirector) {
    pDirector->executeList("エフェクト（前処理）");
    pDirector->executeList("エフェクト（２Ｄ）");
    pDirector->executeList("エフェクト（下画面２Ｄ）");
    pDirector->executeList("エフェクト（後処理）");
}
}  // namespace alExecuteFunction
