#include "Project/Action/ActionAnimCtrl.hpp"

namespace alActionFunction {
    /** @brief Returns the animation name for an action, which defaults to the action's own name. */
    const char* getAnimName(const al::ActionAnimCtrlInfo* pCtrlInfo, const al::ActionAnimDataInfo* pDataInfo) {
        if (pDataInfo->mAnimName != nullptr) {
            return pDataInfo->mAnimName;
        }

        return pCtrlInfo->mActionName;
    }
};
