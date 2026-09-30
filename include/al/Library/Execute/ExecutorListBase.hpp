#pragma once

#include <basis/seadTypes.h>

namespace al {
class ExecutorListBase {
public:
    ExecutorListBase(const char* pListName, const char* pGroupName);
    virtual ~ExecutorListBase() { ; }

    virtual void executeList() const = 0;
    virtual void executeListPaused() const {}
    virtual bool isActive() const = 0;

    const char* mListName;
    const char* mGroupName;
};
}  // namespace al
