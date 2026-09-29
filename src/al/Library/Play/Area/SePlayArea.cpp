#include "Library/Play/AreaObj/SePlayArea.hpp"

#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {

/**
 * @brief Constructs an area that plays a sound effect while the player is inside.
 * @param pName The area name.
 */
SePlayArea::SePlayArea(const char* pName) : AreaObj(pName) {}

/**
 * @brief Reads the sound effect name and the initial enabled state from the placement.
 * @param rInfo The area init info.
 */
void SePlayArea::init(const AreaInitInfo& rInfo) {
    AreaObj::init(rInfo);
    tryGetAreaObjStringArg(&mPlayName, this, "SePlayName");

    bool isStartEnabled = false;
    if (tryGetAreaObjArg(&isStartEnabled, this, "IsStartEnabled") && isStartEnabled) {
        mIsValid = true;
    }
}

}  // namespace al
