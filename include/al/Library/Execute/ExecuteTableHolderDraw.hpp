#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ExecuteOrder;
struct ExecuteSystemInitInfo;
class ExecutorListActorModelDrawBase;
class ExecutorListActorDraw;
class ExecutorListBase;
class ExecutorListFunctor;
class ExecutorListIUseExecutorDraw;
class ExecutorListLayoutDrawBase;
class FunctorBase;
class IUseExecutor;
class LayoutActor;
class LiveActor;

class ExecuteTableHolderDraw {
public:
    ExecuteTableHolderDraw();
    ~ExecuteTableHolderDraw();

    void init(const char* pName, const ExecuteSystemInitInfo& rInfo, const ExecuteOrder* pOrders,
              s32 orderNum);
    ExecutorListActorModelDrawBase*
    registerExecutorListActorModel(ExecutorListActorModelDrawBase* pList);
    ExecutorListActorDraw* registerExecutorListActor(ExecutorListActorDraw* pList);
    ExecutorListLayoutDrawBase* registerExecutorListLayout(ExecutorListLayoutDrawBase* pList);
    ExecutorListIUseExecutorDraw* registerExecutorListUser(ExecutorListIUseExecutorDraw* pList);
    ExecutorListFunctor* registerExecutorListFunctor(ExecutorListFunctor* pList);
    void registerExecutorListAll(ExecutorListBase* pList);
    bool tryRegisterActor(LiveActor* pActor, const char* pListName);
    bool tryRegisterActorModel(LiveActor* pActor, const char* pListName);
    bool tryRegisterLayout(LayoutActor* pLayout, const char* pListName);
    bool tryRegisterUser(IUseExecutor* pUser, const char* pListName);
    bool tryRegisterFunctor(const FunctorBase& rFunctor, const char* pListName);
    void createExecutorListTable();
    void execute() const;
    void executeList(const char* pListName) const;
    bool isActive() const;

    const char* getName() const { return mName; }

    const char* mName = nullptr;
    s32 mActiveListNum = 0;
    ExecutorListBase** mActiveLists = nullptr;
    s32 mListNum = 0;
    s32 mListNumMax = 0;
    ExecutorListBase** mLists = nullptr;
    s32 mActorListNumMax = 0;
    s32 mActorListNum = 0;
    ExecutorListActorDraw** mActorLists = nullptr;
    s32 mActorModelListNumMax = 0;
    s32 mActorModelListNum = 0;
    ExecutorListActorModelDrawBase** mActorModelLists = nullptr;
    s32 mLayoutListNumMax = 0;
    s32 mLayoutListNum = 0;
    ExecutorListLayoutDrawBase** mLayoutLists = nullptr;
    s32 mUserListNumMax = 0;
    s32 mUserListNum = 0;
    ExecutorListIUseExecutorDraw** mUserLists = nullptr;
    s32 mFunctorListNumMax = 0;
    s32 mFunctorListNum = 0;
    ExecutorListFunctor** mFunctorLists = nullptr;
};

static_assert(sizeof(ExecuteTableHolderDraw) == 0x78);
}  // namespace al
