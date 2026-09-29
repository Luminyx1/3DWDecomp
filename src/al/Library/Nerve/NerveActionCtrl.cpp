#include "Library/Nerve/NerveActionCtrl.hpp"

#include "Library/Nerve/NerveAction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * @brief Builds the action table from every action gathered by a collector.
 * @param pCollector The collector holding the linked list of actions.
 */
NerveActionCtrl::NerveActionCtrl(alNerveFunction::NerveActionCollector* pCollector) {
    mNumActions = 0;
    mActions = nullptr;
    mNumActions = pCollector->mNumActions;
    mActions = new NerveAction*[mNumActions];

    NerveAction* pCurrent = pCollector->mStartAction;
    for (s32 i = 0; i < mNumActions; i++) {
        mActions[i] = pCurrent;
        pCurrent = pCurrent->mNextNode;
    }
}

/**
 * @brief Finds an action by its name.
 * @param pName The name of the action to look up.
 * @return The matching action, or nullptr if none has that name.
 */
NerveAction* NerveActionCtrl::findNerve(const char* pName) const {
    for (s32 i = 0; i < mNumActions; i++) {
        NerveAction* pAction = mActions[i];
        if (isEqualString(pAction->getActionName(), pName)) {
            return pAction;
        }
    }

    return nullptr;
}
}  // namespace al
