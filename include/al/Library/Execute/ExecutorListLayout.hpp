#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
class LayoutActor;
struct ExecuteSystemInitInfo;

class ExecutorListLayoutDrawBase : public ExecutorListBase {
public:
    ExecutorListLayoutDrawBase(const char* pListName, s32 capacity, const char* pGroupName,
                               const ExecuteSystemInitInfo& rInfo);

    void executeList() const override;
    bool isActive() const override { return mLayoutNum > 0; }
    virtual void startDraw() const = 0;

    void registerLayout(LayoutActor* pLayout);

    s32 mLayoutNumMax;
    s32 mLayoutNum = 0;
    LayoutActor** mLayouts;
};

class ExecutorListLayoutDrawNormal : public ExecutorListLayoutDrawBase {
public:
    ExecutorListLayoutDrawNormal(const char* pListName, s32 capacity, const char* pGroupName,
                                 const ExecuteSystemInitInfo& rInfo);

    void startDraw() const override;
};

class ExecutorListLayoutUpdate : public ExecutorListBase {
public:
    ExecutorListLayoutUpdate(const char* pListName, s32 capacity, const char* pGroupName);

    void executeList() const override;
    bool isActive() const override { return mLayoutNum > 0; }

    void registerLayout(LayoutActor* pLayout);

    s32 mLayoutNumMax;
    s32 mLayoutNum = 0;
    LayoutActor** mLayouts;
};
}  // namespace al
