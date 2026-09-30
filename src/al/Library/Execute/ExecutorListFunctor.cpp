#include "Library/Execute/ExecutorListFunctor.hpp"

#include "Library/Thread/Functor.hpp"

namespace al {
/**
 * Constructs an empty functor executor list.
 * @param pListName List name.
 * @param pGroupName Group name.
 */
ExecutorListFunctor::ExecutorListFunctor(const char* pListName, const char* pGroupName)
    : ExecutorListBase(pListName, pGroupName) {}

/**
 * Sets the functor to execute.
 * @param rFunctor Functor to clone.
 */
void ExecutorListFunctor::registerFunctor(const FunctorBase& rFunctor) {
    mFunctor = rFunctor.clone();
}

/**
 * Calls the functor.
 */
void ExecutorListFunctor::executeList() const {
    (*mFunctor)();
}
}  // namespace al
