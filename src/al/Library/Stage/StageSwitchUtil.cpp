#include "Library/StageSwitch/StageSwitchFunc.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/Core/IUseStageSwitch.hpp"
#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"
#include "Library/StageSwitch/StageSwitchFunctorListener.hpp"

namespace al {
namespace {
StageSwitchAccesser* getStageSwitchAccesser(const IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchKeeper* keeper = pUser->getStageSwitchKeeper();
    if (!keeper) {
        return nullptr;
    }
    StageSwitchAccesser* accesser = keeper->tryGetStageSwitchAccesser(pLinkName);
    if (!accesser) {
        return nullptr;
    }
    accesser->isEnableRead();
    return accesser;
}
}  // namespace

/**
 * Creates the stage switch keeper of an actor from its placement.
 * @param pUser switch user
 * @param rInfo actor init info
 */
void initStageSwitch(IUseStageSwitch* pUser, const ActorInitInfo& rInfo) {
    initStageSwitch(pUser, rInfo.mStageSwitchDirector, *rInfo.mPlacementInfo);
}

/**
 * Creates the stage switch keeper of a user from a placement.
 * @param pUser switch user
 * @param pDirector stage switch director
 * @param rInfo placement info
 */
void initStageSwitch(IUseStageSwitch* pUser, StageSwitchDirector* pDirector,
                     const PlacementInfo& rInfo) {
    if (pUser->getStageSwitchKeeper()) {
        return;
    }
    pUser->initStageSwitchKeeper();

    if (calcLinkCountClassName(rInfo, "StageSwitch") == 0) {
        return;
    }
    StageSwitchKeeper* keeper = pUser->getStageSwitchKeeper();
    keeper->setUseName(pUser);
    keeper->init(pDirector, rInfo);
}

/**
 * Creates the stage switch keeper of a user if the placement has switch links.
 * @param pUser switch user
 * @param pDirector stage switch director
 * @param rInfo placement info
 * @return true if the placement has switch links
 */
bool tryInitStageSwitch(IUseStageSwitch* pUser, StageSwitchDirector* pDirector,
                        const PlacementInfo& rInfo) {
    if (calcLinkCountClassName(rInfo, "StageSwitch") == 0) {
        return false;
    }
    initStageSwitch(pUser, pDirector, rInfo);
    return true;
}

/**
 * Checks whether a switch link is connected.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if valid
 */
bool isValidStageSwitch(const IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    return accesser && accesser->isValid();
}

/**
 * Checks whether the switch of a link is on.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if on
 */
bool isOnStageSwitch(const IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    return accesser && accesser->isOnSwitch();
}

/**
 * Turns the switch of a link on.
 * @param pUser switch user
 * @param pLinkName link name
 */
void onStageSwitch(IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (accesser) {
        accesser->onSwitch();
    }
}

/**
 * Turns the switch of a link off.
 * @param pUser switch user
 * @param pLinkName link name
 */
void offStageSwitch(IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (accesser) {
        accesser->offSwitch();
    }
}

/**
 * Turns the switch of a link on if it is off.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if the switch was turned on
 */
bool tryOnStageSwitch(IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser || !accesser->isValid() || accesser->isOnSwitch()) {
        return false;
    }
    accesser->onSwitch();
    return true;
}

/**
 * Turns the switch of a link off if it is on.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if the switch was turned off
 */
bool tryOffStageSwitch(IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser || !accesser->isValid() || !accesser->isOnSwitch()) {
        return false;
    }
    accesser->offSwitch();
    return true;
}

/**
 * Turns the switch of a link on if it is off and notifies its listeners immediately.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if the switch was turned on
 */
bool tryOnStageSwitchInstant(IUseStageSwitch* pUser, const char* pLinkName) {
    if (!tryOnStageSwitch(pUser, pLinkName)) {
        return false;
    }
    getStageSwitchAccesser(pUser, pLinkName)->doInstantResponse();
    return true;
}

/**
 * Turns the switch of a link off if it is on and notifies its listeners immediately.
 * @param pUser switch user
 * @param pLinkName link name
 * @return true if the switch was turned off
 */
bool tryOffStageSwitchInstant(IUseStageSwitch* pUser, const char* pLinkName) {
    if (!tryOffStageSwitch(pUser, pLinkName)) {
        return false;
    }
    getStageSwitchAccesser(pUser, pLinkName)->doInstantResponse();
    return true;
}

/**
 * Checks whether two users are connected to the same switch through a link.
 * @param pUser switch user
 * @param pOther other switch user
 * @param pLinkName link name
 * @return true if both use the same switch
 */
bool isSameStageSwitch(const IUseStageSwitch* pUser, const IUseStageSwitch* pOther,
                       const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser) {
        return false;
    }
    StageSwitchAccesser* otherAccesser = getStageSwitchAccesser(pOther, pLinkName);
    if (!otherAccesser) {
        return false;
    }
    return accesser->isEqualSwitch(otherAccesser);
}

/**
 * Gets the switch number of a link.
 * @param pUser switch user
 * @param pLinkName link name
 * @return switch number, or -1 if not connected
 */
s32 findSwitchNo(const IUseStageSwitch* pUser, const char* pLinkName) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser) {
        return -1;
    }
    return accesser->getSwitchNo();
}

/**
 * Checks whether a user is connected to a switch.
 * @param pUser switch user
 * @param switchNo switch number
 * @return true if connected
 */
bool isUsingSwitchNo(const IUseStageSwitch* pUser, s32 switchNo) {
    return pUser->getStageSwitchKeeper()->isUsingSwitchNo(switchNo);
}

/**
 * Checks whether the SwitchAppear link is connected.
 * @param pUser switch user
 * @return true if valid
 */
bool isValidSwitchAppear(const IUseStageSwitch* pUser) {
    return isValidStageSwitch(pUser, "SwitchAppear");
}

/**
 * Checks whether the SwitchAppear switch is on.
 * @param pUser switch user
 * @return true if on
 */
bool isOnSwitchAppear(const IUseStageSwitch* pUser) {
    return isOnStageSwitch(pUser, "SwitchAppear");
}

/**
 * Checks whether the SwitchKill link is connected.
 * @param pUser switch user
 * @return true if valid
 */
bool isValidSwitchKill(const IUseStageSwitch* pUser) {
    return isValidStageSwitch(pUser, "SwitchKill");
}

/**
 * Checks whether the SwitchDeadOn link is connected.
 * @param pUser switch user
 * @return true if valid
 */
bool isValidSwitchDeadOn(const IUseStageSwitch* pUser) {
    return isValidStageSwitch(pUser, "SwitchDeadOn");
}

/**
 * Turns the SwitchDeadOn switch on.
 * @param pUser switch user
 */
void onSwitchDeadOn(IUseStageSwitch* pUser) {
    onStageSwitch(pUser, "SwitchDeadOn");
}

/**
 * Turns the SwitchDeadOn switch off.
 * @param pUser switch user
 */
void offSwitchDeadOn(IUseStageSwitch* pUser) {
    offStageSwitch(pUser, "SwitchDeadOn");
}

/**
 * Turns the SwitchDeadOn switch on if it is off.
 * @param pUser switch user
 * @return true if the switch was turned on
 */
bool tryOnSwitchDeadOn(IUseStageSwitch* pUser) {
    return tryOnStageSwitch(pUser, "SwitchDeadOn");
}

/**
 * Turns the SwitchDeadOn switch off if it is on.
 * @param pUser switch user
 * @return true if the switch was turned off
 */
bool tryOffSwitchDeadOn(IUseStageSwitch* pUser) {
    return tryOffStageSwitch(pUser, "SwitchDeadOn");
}

/**
 * Checks whether the SwitchStart link is connected.
 * @param pUser switch user
 * @return true if valid
 */
bool isValidSwitchStart(const IUseStageSwitch* pUser) {
    return isValidStageSwitch(pUser, "SwitchStart");
}

/**
 * Checks whether the SwitchStart switch is on.
 * @param pUser switch user
 * @return true if on
 */
bool isOnSwitchStart(const IUseStageSwitch* pUser) {
    return isOnStageSwitch(pUser, "SwitchStart");
}

/**
 * Calls a functor when the switch of a link turns on.
 * @param pUser switch user
 * @param pLinkName link name
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOn(IUseStageSwitch* pUser, const char* pLinkName,
                         const FunctorBase& rFunctor) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser || !accesser->isValid()) {
        return false;
    }
    StageSwitchFunctorListener* listener = new StageSwitchFunctorListener();
    listener->setOnFunctor(rFunctor);
    accesser->addListener(listener);
    return true;
}

/**
 * Calls a functor when the switch of a link turns off.
 * @param pUser switch user
 * @param pLinkName link name
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOff(IUseStageSwitch* pUser, const char* pLinkName,
                          const FunctorBase& rFunctor) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser || !accesser->isValid()) {
        return false;
    }
    StageSwitchFunctorListener* listener = new StageSwitchFunctorListener();
    listener->setOffFunctor(rFunctor);
    accesser->addListener(listener);
    return true;
}

/**
 * Calls functors when the switch of a link turns on or off.
 * @param pUser switch user
 * @param pLinkName link name
 * @param rOnFunctor functor to call on
 * @param rOffFunctor functor to call off
 * @return true if the link is connected
 */
bool listenStageSwitchOnOff(IUseStageSwitch* pUser, const char* pLinkName,
                            const FunctorBase& rOnFunctor, const FunctorBase& rOffFunctor) {
    StageSwitchAccesser* accesser = getStageSwitchAccesser(pUser, pLinkName);
    if (!accesser || !accesser->isValid()) {
        return false;
    }
    StageSwitchFunctorListener* listener = new StageSwitchFunctorListener();
    listener->setOnFunctor(rOnFunctor);
    listener->setOffFunctor(rOffFunctor);
    accesser->addListener(listener);
    return true;
}

/**
 * Calls a functor when the SwitchAppear switch turns on.
 * @param pUser switch user
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOnAppear(IUseStageSwitch* pUser, const FunctorBase& rFunctor) {
    return listenStageSwitchOn(pUser, "SwitchAppear", rFunctor);
}

/**
 * Calls functors when the SwitchAppear switch turns on or off.
 * @param pUser switch user
 * @param rOnFunctor functor to call on
 * @param rOffFunctor functor to call off
 * @return true if the link is connected
 */
bool listenStageSwitchOnOffAppear(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                  const FunctorBase& rOffFunctor) {
    return listenStageSwitchOnOff(pUser, "SwitchAppear", rOnFunctor, rOffFunctor);
}

/**
 * Calls a functor when the SwitchKill switch turns on.
 * @param pUser switch user
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOnKill(IUseStageSwitch* pUser, const FunctorBase& rFunctor) {
    return listenStageSwitchOn(pUser, "SwitchKill", rFunctor);
}

/**
 * Calls functors when the SwitchKill switch turns on or off.
 * @param pUser switch user
 * @param rOnFunctor functor to call on
 * @param rOffFunctor functor to call off
 * @return true if the link is connected
 */
bool listenStageSwitchOnOffKill(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                const FunctorBase& rOffFunctor) {
    return listenStageSwitchOnOff(pUser, "SwitchKill", rOnFunctor, rOffFunctor);
}

/**
 * Calls a functor when the SwitchStart switch turns on.
 * @param pUser switch user
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOnStart(IUseStageSwitch* pUser, const FunctorBase& rFunctor) {
    return listenStageSwitchOn(pUser, "SwitchStart", rFunctor);
}

/**
 * Calls functors when the SwitchStart switch turns on or off.
 * @param pUser switch user
 * @param rOnFunctor functor to call on
 * @param rOffFunctor functor to call off
 * @return true if the link is connected
 */
bool listenStageSwitchOnOffStart(IUseStageSwitch* pUser, const FunctorBase& rOnFunctor,
                                 const FunctorBase& rOffFunctor) {
    return listenStageSwitchOnOff(pUser, "SwitchStart", rOnFunctor, rOffFunctor);
}

/**
 * Calls a functor when the SwitchStop switch turns on.
 * @param pUser switch user
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOnStop(IUseStageSwitch* pUser, const FunctorBase& rFunctor) {
    return listenStageSwitchOn(pUser, "SwitchStop", rFunctor);
}

/**
 * Calls a functor when the SwitchGoalItemGetOn switch turns on.
 * @param pUser switch user
 * @param rFunctor functor to call
 * @return true if the link is connected
 */
bool listenStageSwitchOnGoalItemGet(IUseStageSwitch* pUser, const FunctorBase& rFunctor) {
    return listenStageSwitchOn(pUser, "SwitchGoalItemGetOn", rFunctor);
}
}  // namespace al
