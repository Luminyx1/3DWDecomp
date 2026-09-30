#include "Library/LiveActor/ActorAreaFunction.hpp"

#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"

namespace al {
inline SwitchKeepOnAreaGroup::SwitchKeepOnAreaGroup(AreaObjGroup* pGroup) : mGroup(pGroup) {
    mCount = pGroup->mNumAreas;
    if (mCount > 0) {
        mKeepOnAreas = new AreaObj*[mCount];
    }
}

/**
 * Creates a switch keep-on area group from the areas linked to an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @return The group, or nullptr if nothing is linked.
 */
SwitchKeepOnAreaGroup* tryCreateSwitchKeepOnAreaGroup(LiveActor* pActor,
                                                      const ActorInitInfo& rInfo) {
    AreaObjGroup* group = createLinkAreaGroup(pActor, rInfo, "AreaSwitchKeepOn",
                                              "子供スイッチキープエリアグループ",
                                              "子供スイッチキープエリア");
    if (!group) {
        return nullptr;
    }
    return new SwitchKeepOnAreaGroup(group);
}
}  // namespace al
