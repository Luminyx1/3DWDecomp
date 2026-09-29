#include "Library/Execute/ExecutorListFunctor.hpp"
#include "Library/Thread/Functor.hpp"

namespace al {
    /**
     * @brief Creates a functor list with no functor registered.
     * @param pName The name of the list.
     * @param pPauseName The name used while the list is paused, or nullptr to use pName.
     */
    ExecutorListFunctor::ExecutorListFunctor(const char* pName, const char* pPauseName)
        : ExecutorListBase(pName, pPauseName) {}

    /**
     * @brief Stores a copy of a functor to be run by executeList.
     * @param rFunctor The functor to copy.
     */
    void ExecutorListFunctor::registerFunctor(const FunctorBase& rFunctor) {
        mFunctor = rFunctor.clone();
    }

    /** @brief Runs the registered functor. */
    void ExecutorListFunctor::executeList() const {
        (*mFunctor)();
    }
};
