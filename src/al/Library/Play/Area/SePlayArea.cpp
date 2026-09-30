#include "Library/Play/AreaObj/SePlayArea.hpp"

#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/**
 * Constructs a sound play area.
 * @param pName name of the area
 */
SePlayArea::SePlayArea(const char* pName) : AreaObj(pName) {}

/**
 * Initializes the area and reads the sound to play.
 * @param rInfo area init info
 */
void SePlayArea::init(const AreaInitInfo& rInfo) {
    AreaObj::init(rInfo);
    tryGetAreaObjStringArg(&mPlayName, this, "SePlayName");
    bool isStartEnabled = false;
    if (tryGetAreaObjArg(&isStartEnabled, this, "IsStartEnabled") && isStartEnabled) {
        validate();
    }
}
}  // namespace al
