#include "Library/Area/SwitchTriggerArea.hpp"

namespace al {
/**
 * Constructs a switch trigger area.
 * @param pName area name
 */
SwitchTriggerArea::SwitchTriggerArea(const char* pName) : AreaObj(pName) {}

/**
 * Initializes the area.
 * @param rInfo area init info
 */
void SwitchTriggerArea::init(const AreaInitInfo& rInfo) {
    AreaObj::init(rInfo);
}

/**
 * Checks whether a position is inside the area.
 * @param rPos position
 * @return whether the position is inside
 */
bool SwitchTriggerArea::isInVolume(const sead::Vector3f& rPos) const {
    return AreaObj::isInVolume(rPos);
}
}  // namespace al
