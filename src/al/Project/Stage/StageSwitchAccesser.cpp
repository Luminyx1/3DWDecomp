#include "Library/StageSwitch/StageSwitchAccesser.hpp"

#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an accesser that is not connected to a switch.
 */
StageSwitchAccesser::StageSwitchAccesser() = default;

/**
 * Connects the accesser to the switch of a link.
 * @param pDirector stage switch director
 * @param pLinkName link name; names ending in On or Off make the accesser writable
 * @param rId placement id of the switch object
 * @param isDisasterMode true to only react while the disaster camera is active
 * @return true if a switch was assigned
 */
bool StageSwitchAccesser::init(StageSwitchDirector* pDirector, const char* pLinkName,
                               const PlacementId& rId, bool isDisasterMode) {
    mStageSwitchDirector = pDirector;
    mName = pLinkName;
    mPlacementId = new PlacementId(rId);
    mIsDisasterMode = isDisasterMode;

    if (isMatchString(pLinkName, MatchStr("*On")) || isMatchString(pLinkName, MatchStr("*Off"))) {
        mSwitchKind = Write;
    } else {
        mSwitchKind = Read;
    }

    mSwitchNo = mStageSwitchDirector->useSwitch(this);
    return isValid();
}

/**
 * Checks whether a switch is assigned.
 * @return true if valid
 */
bool StageSwitchAccesser::isValid() const {
    return mSwitchNo >= 0;
}

/**
 * Turns the switch on.
 */
void StageSwitchAccesser::onSwitch() {
    if (isValid()) {
        mStageSwitchDirector->onSwitch(this);
    }
}

/**
 * Turns the switch off.
 */
void StageSwitchAccesser::offSwitch() {
    if (isValid()) {
        mStageSwitchDirector->offSwitch(this);
    }
}

/**
 * Checks whether the switch is on.
 * @return true if on
 */
bool StageSwitchAccesser::isOnSwitch() const {
    return mStageSwitchDirector->isOnSwitch(this);
}

/**
 * Gets the stage switch director.
 * @return director
 */
StageSwitchDirector* StageSwitchAccesser::getStageSwitchDirector() const {
    return mStageSwitchDirector;
}

/**
 * Checks whether the switch may be read.
 * @return true if readable
 */
bool StageSwitchAccesser::isEnableRead() const {
    return mSwitchKind == Read || mSwitchKind == Write;
}

/**
 * Checks whether the switch may be written.
 * @return true if writable
 */
bool StageSwitchAccesser::isEnableWrite() const {
    return mSwitchKind == Write;
}

/**
 * Checks whether two accessers use the same switch.
 * @param pOther accesser to compare with
 * @return true if both use the same switch
 */
bool StageSwitchAccesser::isEqualSwitch(const StageSwitchAccesser* pOther) const {
    if (pOther == nullptr) {
        return false;
    }

    return mSwitchNo == pOther->mSwitchNo;
}

/**
 * Notifies the listeners of the switch immediately.
 */
void StageSwitchAccesser::doInstantResponse() {
    mStageSwitchDirector->instantUpdate(this);
}

/**
 * Adds a listener for the switch.
 * @param pListener listener to notify
 */
void StageSwitchAccesser::addListener(StageSwitchListener* pListener) {
    mStageSwitchDirector->addListener(pListener, this);
}
}  // namespace al
