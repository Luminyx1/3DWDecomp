#include "Library/Nerve/NerveAction.hpp"

namespace alNerveFunction {
/**
 * Adds a nerve action to the collector.
 * @param pAction The action to add.
 */
void NerveActionCollector::addNerve(al::NerveAction* pAction) {
    if (mStartAction == nullptr) {
        mStartAction = pAction;
        mEndAction = pAction;
        mNumActions++;
        return;
    }

    mEndAction->mNextNode = pAction;
    mEndAction = pAction;
    mNumActions++;
}

/**
 * Constructs a collector and makes it the current one.
 */
NerveActionCollector::NerveActionCollector() {
    sCurrentCollector = this;
}
}  // namespace alNerveFunction
