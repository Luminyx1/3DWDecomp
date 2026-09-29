#include "Library/Nerve/NerveAction.hpp"

namespace alNerveFunction {
/**
 * @brief Appends a nerve action to the end of the collected action list.
 * @param pAction The action to append.
 */
void NerveActionCollector::addNerve(al::NerveAction* pAction) {
    if (mStartAction != nullptr) {
        mEndAction->mNextNode = pAction;
    } else {
        mStartAction = pAction;
    }

    mEndAction = pAction;
    mNumActions++;
}

/**
 * @brief Constructs an empty collector and makes it the currently active collector.
 */
NerveActionCollector::NerveActionCollector() {
    sCurrentCollector = this;
}
}  // namespace alNerveFunction
