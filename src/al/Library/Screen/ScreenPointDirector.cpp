#include "Library/Screen/ScreenPointDirector.hpp"

#include "Library/Screen/ScreenPointCheckGroup.hpp"
#include "Library/Screen/ScreenPointTarget.hpp"
#include "Library/Screen/ScreenPointer.hpp"

namespace al {
/**
 * Creates the screen point director.
 * @param maxTargets maximum number of targets, or a non-positive value for the default
 */
ScreenPointDirector::ScreenPointDirector(s32 maxTargets) : mCheckGroup(nullptr) {
    if (maxTargets < 1) {
        maxTargets = 0x500;
    }
    mCheckGroup = new ScreenPointCheckGroup(maxTargets);
}

/**
 * Registers a target.
 * @param pTarget target to register
 */
void ScreenPointDirector::registerTarget(ScreenPointTarget* pTarget) {
    mCheckGroup->setTarget(pTarget);
}

/**
 * Assigns the check group of a target.
 * @param pTarget target
 */
void ScreenPointDirector::setCheckGroup(ScreenPointTarget* pTarget) {
    pTarget->setCheckGroup(mCheckGroup);
}
}  // namespace al
