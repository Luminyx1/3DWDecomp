#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ExecuteOrder;
struct ExecuteSystemInitInfo;
class ExecutorListActorExecuteBase;
class ExecutorListBase;
class ExecutorListFunctor;
class ExecutorListIUseExecutorUpdate;
class ExecutorListLayoutUpdate;
class FunctorBase;
class IUseExecutor;
class LayoutActor;
class LiveActor;
class MultiCoreExecutorThreadBase;

class ExecuteTableHolderUpdate {
public:
    ExecuteTableHolderUpdate();
    ~ExecuteTableHolderUpdate();

    void init(const ExecuteSystemInitInfo& rInfo, const ExecuteOrder* pOrders, s32 orderNum,
              bool isUseMultiCore);
    ExecutorListActorExecuteBase* registerExecutorListActor(ExecutorListActorExecuteBase* pList);
    ExecutorListLayoutUpdate* registerExecutorListLayout(ExecutorListLayoutUpdate* pList);
    ExecutorListIUseExecutorUpdate* registerExecutorListUser(ExecutorListIUseExecutorUpdate* pList);
    ExecutorListFunctor* registerExecutorListFunctor(ExecutorListFunctor* pList);
    void registerExecutorListAll(ExecutorListBase* pList);
    void registerActor(LiveActor* pActor, const char* pListName);
    void registerLayout(LayoutActor* pLayout, const char* pListName);
    bool tryRegisterUser(IUseExecutor* pUser, const char* pListName);
    bool tryRegisterFunctor(const FunctorBase& rFunctor, const char* pListName);
    void createExecutorListTable();
    void execute() const;
    void executePaused() const;
    void finishExecute() const;
    void executeList(const char* pListName) const;
    void executeListPaused(const char* pListName) const;

    MultiCoreExecutorThreadBase* mThread = nullptr;
    s32 mActiveListNum = 0;
    ExecutorListBase** mActiveLists = nullptr;
    s32 mListNum = 0;
    s32 mListNumMax = 0;
    ExecutorListBase** mLists = nullptr;
    s32 mActorListNumMax = 0;
    s32 mActorListNum = 0;
    ExecutorListActorExecuteBase** mActorLists = nullptr;
    s32 mLayoutListNumMax = 0;
    s32 mLayoutListNum = 0;
    ExecutorListLayoutUpdate** mLayoutLists = nullptr;
    s32 mUserListNumMax = 0;
    s32 mUserListNum = 0;
    ExecutorListIUseExecutorUpdate** mUserLists = nullptr;
    s32 mFunctorListNumMax = 0;
    s32 mFunctorListNum = 0;
    ExecutorListFunctor** mFunctorLists = nullptr;
};

static_assert(sizeof(ExecuteTableHolderUpdate) == 0x68);
}  // namespace al
