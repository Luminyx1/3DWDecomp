#include "Library/Nerve/NerveAction.hpp"

namespace al {
/**
 * Constructs a nerve action and registers it in the current collector.
 */
NerveAction::NerveAction() {
    alNerveFunction::NerveActionCollector* collector =
        alNerveFunction::NerveActionCollector::sCurrentCollector;
    if (collector->mStartAction == nullptr) {
        collector->mStartAction = this;
        collector->mEndAction = this;
        collector->mNumActions++;
        return;
    }
    collector->mEndAction->mNextNode = this;
    collector->mEndAction = this;
    collector->mNumActions++;
}
}  // namespace al
