#include "Library/MapObj/VisibleSwitchMapParts.hpp"

#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_DECL(VisibleSwitchMapParts, Disappear)
NERVE_DECL(VisibleSwitchMapParts, Appear)
NERVE_DECL(VisibleSwitchMapParts, Show)
NERVE_DECL(VisibleSwitchMapParts, Hide)

NERVES_MAKE_NOSTRUCT(VisibleSwitchMapParts, Disappear, Appear, Show, Hide)
}  // namespace

/**
 * Constructs a map part toggled by switches.
 * @param pName actor name
 */
VisibleSwitchMapParts::VisibleSwitchMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and listens to its appear and disappear switches.
 * @param rInfo actor init info
 */
void VisibleSwitchMapParts::init(const al::ActorInitInfo& rInfo) {
    using VisibleSwitchMapPartsFunctor =
        al::FunctorV0M<VisibleSwitchMapParts*, void (VisibleSwitchMapParts::*)()>;

    al::initActorPoseTRSV(this);
    al::initMapPartsActor(this, rInfo, nullptr, 0);
    al::initNerve(this, &NrvVisibleSwitchMapPartsShow, 0);
    makeActorAppeared();
    if (al::isValidStageSwitch(this, "SwitchAppear") &&
        al::isValidStageSwitch(this, "SwitchDisappear")) {
        return;
    }

    if (al::isValidStageSwitch(this, "SwitchDisappear")) {
        al::listenStageSwitchOnOff(
            this, "SwitchDisappear",
            VisibleSwitchMapPartsFunctor(this, &VisibleSwitchMapParts::startDisappear),
            VisibleSwitchMapPartsFunctor(this, &VisibleSwitchMapParts::startAppear));
        return;
    }

    if (al::isValidStageSwitch(this, "SwitchAppear")) {
        al::listenStageSwitchOnOff(
            this, "SwitchAppear",
            VisibleSwitchMapPartsFunctor(this, &VisibleSwitchMapParts::startAppear),
            VisibleSwitchMapPartsFunctor(this, &VisibleSwitchMapParts::startDisappear));
        al::setNerve(this, &NrvVisibleSwitchMapPartsHide);
    }
}

/**
 * Starts disappearing.
 */
void VisibleSwitchMapParts::startDisappear() {
    if (al::isNerve(this, &NrvVisibleSwitchMapPartsDisappear)) {
        return;
    }

    al::setNerve(this, &NrvVisibleSwitchMapPartsDisappear);
}

/**
 * Starts appearing.
 */
void VisibleSwitchMapParts::startAppear() {
    if (al::isNerve(this, &NrvVisibleSwitchMapPartsAppear)) {
        return;
    }

    al::setNerve(this, &NrvVisibleSwitchMapPartsAppear);
}

/**
 * Ignores screen point collision checks while hidden.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool VisibleSwitchMapParts::receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther,
                                       al::HitSensor* pSelf) {
    if (al::isMsgScreenPointInvalidCollisionParts(pMsg) &&
        al::isNerve(this, &NrvVisibleSwitchMapPartsHide)) {
        return true;
    }

    return false;
}

/**
 * Shows the map part.
 */
void VisibleSwitchMapParts::exeShow() {
    if (al::isFirstStep(this)) {
        al::tryStartAction(this, "Wait");
    }
}

/**
 * Plays the disappear action.
 */
void VisibleSwitchMapParts::exeDisappear() {
    if ((al::isFirstStep(this) && !al::tryStartAction(this, "Disappear")) ||
        al::isActionEnd(this)) {
        al::setNerve(this, &NrvVisibleSwitchMapPartsHide);
    }
}

/**
 * Hides the map part.
 */
void VisibleSwitchMapParts::exeHide() {
    if (al::isFirstStep(this)) {
        al::hideModelIfShow(this);
    }
}

/**
 * Plays the appear action.
 */
void VisibleSwitchMapParts::exeAppear() {
    if (al::isFirstStep(this)) {
        al::showModelIfHide(this);
        if (!al::tryStartAction(this, "Appear")) {
            al::setNerve(this, &NrvVisibleSwitchMapPartsShow);
            return;
        }
    }

    if (al::isActionEnd(this)) {
        al::setNerve(this, &NrvVisibleSwitchMapPartsShow);
    }
}
