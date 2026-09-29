#include "Library/Execute/ExecutorListFunctor.hpp"
#include "Library/Thread/Functor.hpp"

namespace al {
    /** @brief Creates an empty functor list. */
    ExecutorListFunctor::ExecutorListFunctor(const char* pName, const char* pPauseName)
        : ExecutorListBase(pName, pPauseName) {}

    /** @brief Stores a copy of the functor to be run by executeList. */
    void ExecutorListFunctor::registerFunctor(const FunctorBase& rFunctor) {
        mFunctor = rFunctor.clone();
    }

    /** @brief Runs the registered functor. */
    void ExecutorListFunctor::executeList() const {
        (*mFunctor)();
    }
};
