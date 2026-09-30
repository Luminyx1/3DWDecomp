#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
class FunctorBase;

class ExecutorListFunctor : public ExecutorListBase {
public:
    ExecutorListFunctor(const char* pListName, const char* pGroupName);

    void executeList() const override;
    bool isActive() const override { return mFunctor != nullptr; }

    void registerFunctor(const FunctorBase& rFunctor);

    FunctorBase* mFunctor = nullptr;
};
}  // namespace al
