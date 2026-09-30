#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.hpp"

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(SimpleLayoutAppearWaitEnd, Appear);
NERVE_DECL(SimpleLayoutAppearWaitEnd, End);
NERVE_DECL(SimpleLayoutAppearWaitEnd, Wait);
NERVES_MAKE_NOSTRUCT(SimpleLayoutAppearWaitEnd, Appear, End, Wait)
}  // namespace

namespace al {
/**
 * Creates a layout that plays its appear, wait and end actions.
 * @param pName actor name
 * @param pLayoutName layout name
 * @param rInfo layout init info
 * @param pArchiveName archive name, or nullptr to use the layout name
 * @param isLocalized whether to load the localized layout archive
 */
SimpleLayoutAppearWaitEnd::SimpleLayoutAppearWaitEnd(const char* pName, const char* pLayoutName,
                                                     const LayoutInitInfo& rInfo,
                                                     const char* pArchiveName, bool isLocalized)
    : LayoutActor(pName) {
    if (isLocalized) {
        initLayoutActorLocalized(this, rInfo, pLayoutName, pArchiveName);
    } else {
        initLayoutActor(this, rInfo, pLayoutName, pArchiveName);
    }
    initNerve(&NrvSimpleLayoutAppearWaitEndAppear, 0);
}

/**
 * Starts the appear action and makes the layout appear.
 */
void SimpleLayoutAppearWaitEnd::appear() {
    if (!mIsSkipAction) {
        startAction(this, "Appear");
    }
    LayoutActor::appear();
    setNerve(this, &NrvSimpleLayoutAppearWaitEndAppear);
}

/**
 * Starts the end action.
 */
void SimpleLayoutAppearWaitEnd::end() {
    if (isNerve(this, &NrvSimpleLayoutAppearWaitEndEnd)) {
        return;
    }
    setNerve(this, &NrvSimpleLayoutAppearWaitEndEnd);
}

/**
 * Makes the layout appear directly with its wait action.
 */
void SimpleLayoutAppearWaitEnd::startWait() {
    startAction(this, "Wait");
    LayoutActor::appear();
    setNerve(this, &NrvSimpleLayoutAppearWaitEndWait);
}

/**
 * Waits for the appear action to end.
 */
void SimpleLayoutAppearWaitEnd::exeAppear() {
    if (isActionEnd(this)) {
        setNerve(this, &NrvSimpleLayoutAppearWaitEndWait);
    }
}

/**
 * Plays the wait action.
 */
void SimpleLayoutAppearWaitEnd::exeWait() {
    if (isFirstStep(this)) {
        startAction(this, "Wait");
    }
}

/**
 * Plays the end action and kills the layout once it has ended.
 */
void SimpleLayoutAppearWaitEnd::exeEnd() {
    if (isFirstStep(this) && !mIsSkipAction) {
        startAction(this, "End");
    }
    if (isActionEnd(this)) {
        kill();
    }
}

/**
 * Checks whether the layout is waiting.
 * @return whether the layout is in its wait state
 */
bool SimpleLayoutAppearWaitEnd::isWait() const {
    return isNerve(this, &NrvSimpleLayoutAppearWaitEndWait);
}

/**
 * Checks whether the layout is ending.
 * @return whether the layout is in its end state
 */
bool SimpleLayoutAppearWaitEnd::isEnd() const {
    return isNerve(this, &NrvSimpleLayoutAppearWaitEndEnd);
}

/**
 * Creates a plain layout actor.
 * @param pName actor name
 * @param pLayoutName layout name
 * @param rInfo layout init info
 * @param pArchiveName archive name, or nullptr to use the layout name
 * @return the layout actor
 */
LayoutActor* createSimpleLayout(const char* pName, const char* pLayoutName,
                                const LayoutInitInfo& rInfo, const char* pArchiveName) {
    LayoutActor* actor = new LayoutActor(pName);
    initLayoutActor(actor, rInfo, pLayoutName, pArchiveName);
    return actor;
}
}  // namespace al
