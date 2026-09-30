#include "Library/Nerve/NerveActionCtrl.hpp"

#include "Library/Nerve/NerveAction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs a nerve action controller from the actions of a collector.
 * @param pCollector The collector holding the actions.
 */
NerveActionCtrl::NerveActionCtrl(alNerveFunction::NerveActionCollector* pCollector) {
    mNumActions = pCollector->mNumActions;
    mActions = new NerveAction*[mNumActions];
    NerveAction* action = pCollector->mStartAction;

    for (s32 i = 0; i < mNumActions; i++) {
        mActions[i] = action;
        action = action->mNextNode;
    }
}

/**
 * Finds a nerve action by name.
 * @param pName The action name.
 * @return The action, or nullptr if none has that name.
 */
NerveAction* NerveActionCtrl::findNerve(const char* pName) const {
    for (s32 i = 0; i < mNumActions; i++) {
        NerveAction* action = mActions[i];

        if (isEqualString(action->getActionName(), pName)) {
            return action;
        }
    }

    return nullptr;
}
}  // namespace al
