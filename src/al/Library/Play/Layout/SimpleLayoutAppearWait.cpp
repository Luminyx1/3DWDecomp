#include "Library/Play/Layout/SimpleLayoutAppearWait.hpp"

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(SimpleLayoutAppearWait, Appear);
NERVE_DECL(SimpleLayoutAppearWait, Wait);
NERVES_MAKE_NOSTRUCT(SimpleLayoutAppearWait, Appear, Wait)
}  // namespace

namespace al {
/**
 * Creates a layout that plays its appear action and then its wait action.
 * @param pName actor name
 * @param pLayoutName layout name
 * @param rInfo layout init info
 * @param pArchiveName archive name, or nullptr to use the layout name
 */
SimpleLayoutAppearWait::SimpleLayoutAppearWait(const char* pName, const char* pLayoutName,
                                               const LayoutInitInfo& rInfo,
                                               const char* pArchiveName)
    : LayoutActor(pName) {
    initLayoutActor(this, rInfo, pLayoutName, pArchiveName);
    initNerve(&NrvSimpleLayoutAppearWaitAppear, 0);
}

/**
 * Starts the appear action and makes the layout appear.
 */
void SimpleLayoutAppearWait::appear() {
    startAction(this, "Appear");
    LayoutActor::appear();
    setNerve(this, &NrvSimpleLayoutAppearWaitAppear);
}

/**
 * Makes the layout appear directly with its wait action.
 */
void SimpleLayoutAppearWait::appear2() {
    startAction(this, "Wait");
    LayoutActor::appear();
    setNerve(this, &NrvSimpleLayoutAppearWaitAppear);
}

/**
 * Waits for the appear action to end.
 */
void SimpleLayoutAppearWait::exeAppear() {
    if (isActionEnd(this)) {
        setNerve(this, &NrvSimpleLayoutAppearWaitWait);
    }
}

/**
 * Plays the wait action.
 */
void SimpleLayoutAppearWait::exeWait() {
    if (isFirstStep(this)) {
        startAction(this, "Wait");
    }
}
}  // namespace al
