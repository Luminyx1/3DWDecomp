#include "Project/Action/ActionAnimCtrl.hpp"

namespace alActionFunction {
    /**
     * @brief Gets the name of the animation an action plays.
     * @param pCtrlInfo The action's animation control info.
     * @param pDataInfo The action's animation data.
     * @return The animation name from pDataInfo, or the action's own name when it has none.
     */
    const char* getAnimName(const al::ActionAnimCtrlInfo* pCtrlInfo, const al::ActionAnimDataInfo* pDataInfo) {
        if (pDataInfo->mAnimName != nullptr) {
            return pDataInfo->mAnimName;
        }

        return pCtrlInfo->mActionName;
    }
};
