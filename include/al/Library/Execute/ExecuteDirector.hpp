#pragma once

#include <basis/seadTypes.h>

namespace al {
class ExecuteRequestKeeper;
class ExecuteTableHolderDraw;
class ExecuteTableHolderUpdate;
struct ExecuteSystemInitInfo;
class FunctorBase;
class IUseExecutor;
class LayoutActor;
class LiveActor;

class ExecuteDirector {
public:
    ExecuteDirector(s32 requestCount, bool isUseMultiCore);
    ~ExecuteDirector();

    void init(const ExecuteSystemInitInfo& rInfo);
    void registerActorUpdate(LiveActor* pActor, const char* pListName);
    void registerActorDraw(LiveActor* pActor, const char* pListName);
    void registerActorModelDraw(LiveActor* pActor, const char* pListName);
    void registerLayoutUpdate(LayoutActor* pLayout, const char* pListName);
    void registerLayoutDraw(LayoutActor* pLayout, const char* pListName);
    void registerUser(IUseExecutor* pUser, const char* pListName);
    void registerFunctor(const FunctorBase& rFunctor, const char* pListName);
    void registerFunctorDraw(const FunctorBase& rFunctor, const char* pListName);
    void createExecutorListTable();
    void execute() const;
    void finishExecute() const;
    void executeList(const char* pListName) const;
    void executeListPaused(const char* pListName) const;
    void executeListStall(const char* pListName) const;
    void draw(const char* pTableName) const;
    void drawList(const char* pTableName, const char* pListName) const;
    bool isActiveDraw(const char* pTableName) const;

    s32 mRequestCount;
    ExecuteTableHolderUpdate* mUpdateTable = nullptr;
    s32 mDrawTableNum = 0;
    ExecuteTableHolderDraw** mDrawTables = nullptr;
    ExecuteRequestKeeper* mRequestKeeper = nullptr;
    bool mIsUseMultiCore;
};

static_assert(sizeof(ExecuteDirector) == 0x30);
}  // namespace al
