#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
    class FunctorBase;

    /// An executor list that runs a single registered functor.
    class ExecutorListFunctor : public ExecutorListBase {
    public:
        ExecutorListFunctor(const char* pName, const char* pPauseName);

        void executeList() const override;
        bool isActive() const override { return mFunctor != nullptr; }

        void registerFunctor(const FunctorBase& rFunctor);

        FunctorBase* mFunctor = nullptr;    // _18
    };

    static_assert(sizeof(ExecutorListFunctor) == 0x20, "ExecutorListFunctor size");
};
