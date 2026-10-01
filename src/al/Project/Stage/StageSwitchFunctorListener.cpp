#include "Library/StageSwitch/StageSwitchFunctorListener.hpp"

#include "Library/Thread/Functor.hpp"

namespace al {
/**
 * Constructs a listener without functors.
 */
StageSwitchFunctorListener::StageSwitchFunctorListener() = default;

/**
 * Sets the functor called when the switch turns on.
 * @param rFunctor functor to copy
 */
void StageSwitchFunctorListener::setOnFunctor(const FunctorBase& rFunctor) {
    mOnFunctor = rFunctor.clone();
}

/**
 * Sets the functor called when the switch turns off.
 * @param rFunctor functor to copy
 */
void StageSwitchFunctorListener::setOffFunctor(const FunctorBase& rFunctor) {
    mOffFunctor = rFunctor.clone();
}

/**
 * Calls the on functor.
 */
void StageSwitchFunctorListener::listenOn() {
    if (mOnFunctor != nullptr) {
        (*mOnFunctor)();
    }
}

/**
 * Calls the off functor.
 */
void StageSwitchFunctorListener::listenOff() {
    if (mOffFunctor != nullptr) {
        (*mOffFunctor)();
    }
}
}  // namespace al
