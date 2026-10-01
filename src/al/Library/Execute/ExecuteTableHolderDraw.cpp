#include "Library/Execute/ExecuteTableHolderDraw.hpp"

#include "Library/Execute/ExecuteOrder.hpp"
#include "Library/Execute/ExecutorListActorExecute.hpp"
#include "Library/Execute/ExecutorListActorModelDraw.hpp"
#include "Library/Execute/ExecutorListFunctor.hpp"
#include "Library/Execute/ExecutorListLayout.hpp"
#include "Library/Execute/ExecutorListUser.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Execute/ExecuteFunction.hpp"

namespace al {
using alExecutorFunction::isListName;

static inline bool isEqualGroup(const ExecuteOrder& rOrder, const char* pListName) {
    return isEqualString(rOrder.mExecuteGroup, pListName);
}

static inline bool isDrawListActorModel(const ExecuteOrder& rOrder) {
    return isEqualGroup(rOrder, "ActorModelDraw") ||
           isEqualGroup(rOrder, "ActorModelDrawXlu") ||
           isEqualGroup(rOrder, "ActorModelDrawCharacter") ||
           isEqualGroup(rOrder, "ActorModelDrawCharacterOpt") ||
           isEqualGroup(rOrder, "ActorModelDrawCharacterXlu") ||
           isEqualGroup(rOrder, "ActorModelDrawCubeMap") ||
           isEqualGroup(rOrder, "ActorModelDrawDepthOnly") ||
           isEqualGroup(rOrder, "ActorModelDrawDepthForce") ||
           isEqualGroup(rOrder, "ActorModelDrawDepthForceOpt") ||
           isEqualGroup(rOrder, "ActorModelDrawDepthShadow") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferred") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredOpa") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredCharacter") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredPlayer") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredCharacterOpa") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredFootPrint") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredEcho") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredSilhouette") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredSilhouetteRide") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredSky") ||
           isEqualGroup(rOrder, "ActorModelDrawInvincible") ||
           isEqualGroup(rOrder, "ActorModelDrawPostEffectMask") ||
           isEqualGroup(rOrder, "ActorModelDrawDeferredSSD") ||
           isEqualGroup(rOrder, "ActorModelDrawHeightMap");
}

static inline bool isDrawListLayout(const ExecuteOrder& rOrder) {
    return isEqualGroup(rOrder, "LayoutDraw") || isEqualGroup(rOrder, "LayoutDrawBottom");
}

/**
 * Constructs an empty draw table.
 */
ExecuteTableHolderDraw::ExecuteTableHolderDraw() = default;

ExecuteTableHolderDraw::~ExecuteTableHolderDraw() {
    for (s32 i = 0; i < mActorModelListNum; i++) {
        if (mActorModelLists[i] != nullptr) {
            delete mActorModelLists[i];
        }
    }
}

/**
 * Creates the executor lists of a draw table from its execute orders.
 * @param pName Table name.
 * @param rInfo Execute system init info.
 * @param pOrders Execute orders.
 * @param orderNum Number of execute orders.
 */
void ExecuteTableHolderDraw::init(const char* pName, const ExecuteSystemInitInfo& rInfo,
                                  const ExecuteOrder* pOrders, s32 orderNum) {
    mName = pName;
    mListNumMax = orderNum;
    mLists = new ExecutorListBase*[orderNum];

    s32 actorNum = 0;

    for (s32 i = 0; i < orderNum; i++) {
        actorNum += isEqualGroup(pOrders[i], "ActorDraw");
    }

    mActorListNumMax = actorNum;
    mActorLists = new ExecutorListActorDraw*[actorNum];

    s32 actorModelNum = 0;

    for (s32 i = 0; i < orderNum; i++) {
        if (isDrawListActorModel(pOrders[i])) {
            actorModelNum++;
        }
    }

    mActorModelListNumMax = actorModelNum;
    mActorModelLists = new ExecutorListActorModelDrawBase*[actorModelNum];

    s32 layoutNum = 0;

    for (s32 i = 0; i < orderNum; i++) {
        if (isDrawListLayout(pOrders[i])) {
            layoutNum++;
        }
    }

    mLayoutListNumMax = layoutNum;
    mLayoutLists = new ExecutorListLayoutDrawBase*[layoutNum];

    mUserListNumMax = alExecutorFunction::calcExecutorListNumMax(pOrders, orderNum, "Draw");
    mUserLists = new ExecutorListIUseExecutorDraw*[mUserListNumMax];

    mFunctorListNumMax = alExecutorFunction::calcExecutorListNumMax(pOrders, orderNum, "Functor");
    mFunctorLists = new ExecutorListFunctor*[mFunctorListNumMax];

    for (s64 i = 0; i < mListNumMax; i++) {
        const ExecuteOrder& order = pOrders[i];
        ExecutorListBase* list;

        if (isListName(order, "ActorDraw")) {
            list = registerExecutorListActor(
                new ExecutorListActorDraw(order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "LayoutDraw")) {
            list = registerExecutorListLayout(new ExecutorListLayoutDrawNormal(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "Draw")) {
            list = registerExecutorListUser(new ExecutorListIUseExecutorDraw(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "Functor")) {
            list = registerExecutorListFunctor(
                new ExecutorListFunctor(order.mListName, order.mGroupName));
        } else if (isListName(order, "ActorModelDraw")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForward(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawXlu")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForwardXlu(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawPostEffectMask")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForwardPostEffectMask(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawCharacter")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForwardCharacter(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawCharacterOpt")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForwardCharacterOpt(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawCharacterXlu")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawForwardCharacterXlu(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawCubeMap")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawCubeMap(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferred")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferred(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredOpa")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredOpa(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredCharacter")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredCharacter(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredPlayer")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredPlayer(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredSSD")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredSSD(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredCharacterOpa")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredCharacterOpa(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDepthOnly")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDepthOnly(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDepthForce")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDepthForce(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDepthForceOpt")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDepthForceOpt(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDepthShadow")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDepthShadow(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredEcho")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredEcho(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredSilhouette")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredSilhouette(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredSilhouetteRide")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredSilhouetteRide(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredFootPrint")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredFootPrint(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawDeferredSky")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawDeferredSky(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else if (isListName(order, "ActorModelDrawInvincible")) {
            list = registerExecutorListActorModel(new ExecutorListActorModelDrawInvincible(
                order.mListName, order.mListMaxSize, order.mGroupName, rInfo));
        } else {
            continue;
        }

        registerExecutorListAll(list);
    }
}

/**
 * Adds an actor model draw list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListActorModelDrawBase*
ExecuteTableHolderDraw::registerExecutorListActorModel(ExecutorListActorModelDrawBase* pList) {
    mActorModelLists[mActorModelListNum] = pList;
    mActorModelListNum++;
    return pList;
}

/**
 * Adds an actor draw list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListActorDraw* ExecuteTableHolderDraw::registerExecutorListActor(ExecutorListActorDraw* pList) {
    mActorLists[mActorListNum] = pList;
    mActorListNum++;
    return pList;
}

/**
 * Adds a layout draw list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListLayoutDrawBase*
ExecuteTableHolderDraw::registerExecutorListLayout(ExecutorListLayoutDrawBase* pList) {
    mLayoutLists[mLayoutListNum] = pList;
    mLayoutListNum++;
    return pList;
}

/**
 * Adds an executor user draw list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListIUseExecutorDraw*
ExecuteTableHolderDraw::registerExecutorListUser(ExecutorListIUseExecutorDraw* pList) {
    mUserLists[mUserListNum] = pList;
    mUserListNum++;
    return pList;
}

/**
 * Adds a functor list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListFunctor* ExecuteTableHolderDraw::registerExecutorListFunctor(ExecutorListFunctor* pList) {
    mFunctorLists[mFunctorListNum] = pList;
    mFunctorListNum++;
    return pList;
}

/**
 * Adds a list to the table of all lists.
 * @param pList The list.
 */
void ExecuteTableHolderDraw::registerExecutorListAll(ExecutorListBase* pList) {
    mLists[mListNum] = pList;
    mListNum++;
}

/**
 * Registers an actor to all actor draw lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderDraw::tryRegisterActor(LiveActor* pActor, const char* pListName) {
    bool isRegistered = false;

    for (s32 i = 0; i < mActorListNum; i++) {
        ExecutorListActorDraw* list = mActorLists[i];

        if (isEqualString(list->mListName, pListName)) {
            list->registerActor(pActor);
            isRegistered = true;
        }
    }

    return isRegistered;
}

/**
 * Registers an actor model to all actor model draw lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderDraw::tryRegisterActorModel(LiveActor* pActor, const char* pListName) {
    bool isRegistered = false;

    for (s32 i = 0; i < mActorModelListNum; i++) {
        ExecutorListActorModelDrawBase* list = mActorModelLists[i];

        if (isEqualString(list->mListName, pListName)) {
            list->registerActorModel(pActor);
            isRegistered = true;
        }
    }

    return isRegistered;
}

/**
 * Registers a layout to all layout draw lists with the given name.
 * @param pLayout The layout.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderDraw::tryRegisterLayout(LayoutActor* pLayout, const char* pListName) {
    bool isRegistered = false;

    for (s32 i = 0; i < mLayoutListNum; i++) {
        ExecutorListLayoutDrawBase* list = mLayoutLists[i];

        if (isEqualString(list->mListName, pListName)) {
            list->registerLayout(pLayout);
            isRegistered = true;
        }
    }

    return isRegistered;
}

/**
 * Registers an executor user to all user draw lists with the given name.
 * @param pUser The user.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderDraw::tryRegisterUser(IUseExecutor* pUser, const char* pListName) {
    bool isRegistered = false;

    for (s32 i = 0; i < mUserListNum; i++) {
        ExecutorListIUseExecutorDraw* list = mUserLists[i];

        if (isEqualString(list->mListName, pListName)) {
            list->registerUser(pUser);
            isRegistered = true;
        }
    }

    return isRegistered;
}

/**
 * Registers a functor to all functor lists with the given name.
 * @param rFunctor The functor.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderDraw::tryRegisterFunctor(const FunctorBase& rFunctor, const char* pListName) {
    bool isRegistered = false;

    for (s32 i = 0; i < mFunctorListNum; i++) {
        ExecutorListFunctor* list = mFunctorLists[i];

        if (isEqualString(list->mListName, pListName)) {
            list->registerFunctor(rFunctor);
            isRegistered = true;
        }
    }

    return isRegistered;
}

/**
 * Creates the executor tables and collects the active lists.
 */
void ExecuteTableHolderDraw::createExecutorListTable() {
    for (s32 i = 0; i < mActorListNum; i++) {
        mActorLists[i]->createList();
    }

    for (s32 i = 0; i < mActorModelListNum; i++) {
        mActorModelLists[i]->createList();
    }

    mActiveListNum = 0;

    for (s32 i = 0; i < mListNum; i++) {
        if (mLists[i]->isActive()) {
            mActiveListNum++;
        }
    }

    mActiveLists = new ExecutorListBase*[mActiveListNum];
    s32 index = 0;

    for (s32 i = 0; i < mListNum; i++) {
        ExecutorListBase* list = mLists[i];

        if (list->isActive()) {
            mActiveLists[index++] = list;
        }
    }
}

/**
 * Executes all active lists.
 */
void ExecuteTableHolderDraw::execute() const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        mActiveLists[i]->executeList();
    }
}

/**
 * Executes the active lists with the given name.
 * @param pListName List name.
 */
void ExecuteTableHolderDraw::executeList(const char* pListName) const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        if (isEqualString(mActiveLists[i]->mListName, pListName)) {
            mActiveLists[i]->executeList();
        }
    }
}

/**
 * Checks whether all active lists are still active.
 * @return True if there are active lists and all are active.
 */
bool ExecuteTableHolderDraw::isActive() const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        if (!mActiveLists[i]->isActive()) {
            return false;
        }
    }

    return mActiveListNum > 0;
}
}  // namespace al
