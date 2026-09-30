#include "Project/Action/Common/ActionAnimInfo.hpp"

namespace alActionFunction {

/**
 * Gets the animation name of an action entry, falling back to the action name.
 * @param pCtrlInfo Action whose name is the fallback.
 * @param pDataInfo Animation entry of the action.
 * @return Animation name.
 */
const char* getAnimName(const al::ActionAnimCtrlInfo* pCtrlInfo,
                        const al::ActionAnimDataInfo* pDataInfo) {
    const char* animName = pDataInfo->actionName;
    if (!animName) {
        animName = pCtrlInfo->actionName;
    }

    return animName;
}

}  // namespace alActionFunction
