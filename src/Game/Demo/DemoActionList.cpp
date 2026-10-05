#include "Demo/DemoActionList.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace DemoSceneActorFunction {
bool isHideAction(const char* pActionName);
bool isShowAction(const char* pActionName);
}

/**
 * @brief Reads the three numbered action arguments for a demo actor.
 * @param rInfo Placement arguments containing the optional action names.
 * @param pArgPrefix Prefix followed by a one-based action number.
 */
DemoActionList::DemoActionList(const al::ActorInitInfo& rInfo, const char* pArgPrefix)
    : mActionCount(3) {
    mActionNames = new const char*[mActionCount];
    for (int i = 0; i < mActionCount; ++i) {
        mActionNames[i] = nullptr;
        al::tryGetStringArg(&mActionNames[i], rInfo,
                            al::StringTmp<64>("%s%d", pArgPrefix, i + 1).cstr());
    }
}

/**
 * @brief Gets an optional action name by its zero-based index.
 * @param index Action index; out-of-range values are accepted.
 * @return The configured name, or nullptr for an absent or invalid entry.
 */
const char* DemoActionList::getActionName(int index) const {
    if (index < 0 || index >= mActionCount) {
        return nullptr;
    }
    return mActionNames[index];
}

/**
 * @brief Applies a visibility command or starts the selected actor action.
 * @param pActor Actor whose model and action are updated.
 * @param index Zero-based action index; absent entries do nothing.
 */
void DemoActionList::startAction(al::LiveActor* pActor, int index) {
    const char* pActionName = getActionName(index);
    if (DemoSceneActorFunction::isHideAction(pActionName)) {
        if (!al::isHideModel(pActor)) {
            al::hideModel(pActor);
        }
    } else if (DemoSceneActorFunction::isShowAction(pActionName)) {
        if (al::isHideModel(pActor)) {
            al::showModel(pActor);
        }
    } else if (pActionName) {
        if (al::isHideModel(pActor)) {
            al::showModel(pActor);
        }
        al::startAction(pActor, pActionName);
    }
}
