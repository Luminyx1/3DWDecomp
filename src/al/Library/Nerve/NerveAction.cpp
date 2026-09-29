#include "Library/Nerve/NerveAction.hpp"

namespace al {
/**
 * @brief Constructs a nerve action and appends it to the currently active action collector.
 */
NerveAction::NerveAction() {
    alNerveFunction::NerveActionCollector* pCollector =
        alNerveFunction::NerveActionCollector::sCurrentCollector;

    if (pCollector->mStartAction != nullptr) {
        pCollector->mEndAction->mNextNode = this;
    } else {
        pCollector->mStartAction = this;
    }

    pCollector->mEndAction = this;
    pCollector->mNumActions++;
}
}  // namespace al
