#include "Library/Execute/ExecuteTableHolderUpdate.hpp"

#include <thread/seadThread.h>

#include "Library/Execute/ExecuteOrder.hpp"
#include "Library/Execute/ExecutorListActorExecute.hpp"
#include "Library/Execute/ExecutorListFunctor.hpp"
#include "Library/Execute/ExecutorListLayout.hpp"
#include "Library/Execute/ExecutorListUser.hpp"
#include "Library/Execute/MultiCoreExecutorThread.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Execute/ExecuteFunction.hpp"

namespace al {
using alExecutorFunction::isListName;

static inline bool isUpdateListActor(const ExecuteOrder& rOrder) {
    return isEqualString(rOrder.mExecuteGroup, "ActorMovement") ||
           isEqualString(rOrder.mExecuteGroup, "ActorCalcAnim") ||
           isEqualString(rOrder.mExecuteGroup, "ActorMovementCalcAnim") ||
           isEqualString(rOrder.mExecuteGroup, "ActorModelUpdate");
}

/**
 * Constructs an empty update table.
 */
ExecuteTableHolderUpdate::ExecuteTableHolderUpdate() = default;

ExecuteTableHolderUpdate::~ExecuteTableHolderUpdate() {
    if (mThread) {
        delete mThread;
    }
}

/**
 * Creates the executor lists of an update table from its execute orders.
 * @param rInfo Execute system init info.
 * @param pOrders Execute orders.
 * @param orderNum Number of execute orders.
 * @param isUseMultiCore Whether to calculate actor anims on another core.
 */
void ExecuteTableHolderUpdate::init(const ExecuteSystemInitInfo& rInfo, const ExecuteOrder* pOrders,
                                    s32 orderNum, bool isUseMultiCore) {
    mListNumMax = orderNum;
    mLists = new ExecutorListBase*[orderNum];

    s32 actorNum = 0;
    for (s32 i = 0; i < orderNum; i++) {
        if (isUpdateListActor(pOrders[i])) {
            actorNum++;
        }
    }

    mActorListNumMax = actorNum;
    mActorLists = new ExecutorListActorExecuteBase*[actorNum];

    mLayoutListNumMax =
        alExecutorFunction::calcExecutorListNumMax(pOrders, orderNum, "LayoutUpdate");
    mLayoutLists = new ExecutorListLayoutUpdate*[mLayoutListNumMax];

    mUserListNumMax = alExecutorFunction::calcExecutorListNumMax(pOrders, orderNum, "Execute");
    mUserLists = new ExecutorListIUseExecutorUpdate*[mUserListNumMax];

    mFunctorListNumMax = alExecutorFunction::calcExecutorListNumMax(pOrders, orderNum, "Functor");
    mFunctorLists = new ExecutorListFunctor*[mFunctorListNumMax];

    if (isUseMultiCore) {
        mThread = new MultiCoreExecutorThreadActorCalcAnim(
            0x100, sead::Thread::cDefaultPriority - 1, sead::CoreId::cSub1);
    }

    for (s64 i = 0; i < mListNumMax; i++) {
        const ExecuteOrder& order = pOrders[i];
        ExecutorListBase* list = nullptr;

        if (isListName(order, "ActorMovement")) {
            list = registerExecutorListActor(new ExecutorListActorMovement(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "ActorCalcAnim")) {
            list = registerExecutorListActor(new ExecutorListActorCalcAnim(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "ActorMovementCalcAnim")) {
            list = registerExecutorListActor(new ExecutorListActorMovementCalcAnim(
                order.mListName, order.mListMaxSize, order.mGroupName, mThread));
        } else if (isListName(order, "ActorModelUpdate")) {
            list = registerExecutorListActor(new ExecutorListActorModelUpdate(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "LayoutUpdate")) {
            list = registerExecutorListLayout(new ExecutorListLayoutUpdate(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "Execute")) {
            list = registerExecutorListUser(new ExecutorListIUseExecutorUpdate(
                order.mListName, order.mListMaxSize, order.mGroupName));
        } else if (isListName(order, "Functor")) {
            list = registerExecutorListFunctor(
                new ExecutorListFunctor(order.mListName, order.mGroupName));
        }

        registerExecutorListAll(list);
    }
}

/**
 * Adds an actor list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListActorExecuteBase* ExecuteTableHolderUpdate::registerExecutorListActor(ExecutorListActorExecuteBase* pList) {
    mActorLists[mActorListNum] = pList;
    mActorListNum++;
    return pList;
}

/**
 * Adds a layout list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListLayoutUpdate* ExecuteTableHolderUpdate::registerExecutorListLayout(ExecutorListLayoutUpdate* pList) {
    mLayoutLists[mLayoutListNum] = pList;
    mLayoutListNum++;
    return pList;
}

/**
 * Adds an executor user list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListIUseExecutorUpdate* ExecuteTableHolderUpdate::registerExecutorListUser(ExecutorListIUseExecutorUpdate* pList) {
    mUserLists[mUserListNum] = pList;
    mUserListNum++;
    return pList;
}

/**
 * Adds a functor list.
 * @param pList The list.
 * @return The list.
 */
ExecutorListFunctor* ExecuteTableHolderUpdate::registerExecutorListFunctor(ExecutorListFunctor* pList) {
    mFunctorLists[mFunctorListNum] = pList;
    mFunctorListNum++;
    return pList;
}

/**
 * Adds a list to the table of all lists.
 * @param pList The list.
 */
void ExecuteTableHolderUpdate::registerExecutorListAll(ExecutorListBase* pList) {
    mLists[mListNum] = pList;
    mListNum++;
}

/**
 * Registers an actor to all actor lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 */
void ExecuteTableHolderUpdate::registerActor(LiveActor* pActor, const char* pListName) {
    for (s32 i = 0; i < mActorListNum; i++) {
        ExecutorListActorExecuteBase* list = mActorLists[i];
        if (isEqualString(list->mListName, pListName)) {
            list->registerActor(pActor);
        }
    }
}

/**
 * Registers a layout to all layout lists with the given name.
 * @param pLayout The layout.
 * @param pListName List name.
 */
void ExecuteTableHolderUpdate::registerLayout(LayoutActor* pLayout, const char* pListName) {
    for (s32 i = 0; i < mLayoutListNum; i++) {
        ExecutorListLayoutUpdate* list = mLayoutLists[i];
        if (isEqualString(list->mListName, pListName)) {
            list->registerLayout(pLayout);
        }
    }
}

/**
 * Registers an executor user to all user lists with the given name.
 * @param pUser The user.
 * @param pListName List name.
 * @return True if a list was found.
 */
bool ExecuteTableHolderUpdate::tryRegisterUser(IUseExecutor* pUser, const char* pListName) {
    bool isRegistered = false;
    for (s32 i = 0; i < mUserListNum; i++) {
        ExecutorListIUseExecutorUpdate* list = mUserLists[i];
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
bool ExecuteTableHolderUpdate::tryRegisterFunctor(const FunctorBase& rFunctor,
                                                  const char* pListName) {
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
void ExecuteTableHolderUpdate::createExecutorListTable() {
    for (s32 i = 0; i < mActorListNum; i++) {
        mActorLists[i]->createList();
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
void ExecuteTableHolderUpdate::execute() const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        mActiveLists[i]->executeList();
    }
}

/**
 * Executes all active lists while paused.
 */
void ExecuteTableHolderUpdate::executePaused() const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        mActiveLists[i]->executeListPaused();
    }
}

/**
 * Waits for the anim calculation thread.
 */
void ExecuteTableHolderUpdate::finishExecute() const {
    if (mThread) {
        mThread->waitDoneAll();
    }
}

/**
 * Executes the active lists with the given name.
 * @param pListName List name.
 */
void ExecuteTableHolderUpdate::executeList(const char* pListName) const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        if (isEqualString(mActiveLists[i]->mListName, pListName)) {
            mActiveLists[i]->executeList();
        }
    }
}

/**
 * Executes the active lists with the given name while paused.
 * @param pListName List name.
 */
void ExecuteTableHolderUpdate::executeListPaused(const char* pListName) const {
    for (s32 i = 0; i < mActiveListNum; i++) {
        if (isEqualString(mActiveLists[i]->mListName, pListName)) {
            mActiveLists[i]->executeListPaused();
        }
    }
}
}  // namespace al
