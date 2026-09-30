#include "Library/Play/Layout/WipeSimple.hpp"

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(WipeSimple, Close);
NERVE_DECL(WipeSimple, CloseEnd);
NERVE_DECL(WipeSimple, Open);
NERVE_DECL(WipeSimple, DelayOpen);
NERVES_MAKE_NOSTRUCT(WipeSimple, Close, CloseEnd, Open, DelayOpen)
}  // namespace

namespace al {
/**
 * Creates a simple wipe layout.
 * @param pName actor name
 * @param pLayoutName layout name
 * @param rInfo layout init info
 * @param pArchiveName archive name, or nullptr to use the layout name
 */
WipeSimple::WipeSimple(const char* pName, const char* pLayoutName, const LayoutInitInfo& rInfo,
                       const char* pArchiveName)
    : LayoutActor(pName) {
    initLayoutActor(this, rInfo, pLayoutName, pArchiveName);
    initNerve(&NrvWipeSimpleClose, 0);
}

/**
 * Starts closing the wipe.
 * @param frames close duration in frames, or a non-positive value for the action's own length
 */
void WipeSimple::startClose(s32 frames) {
    mFrames = frames;
    startAction(this, "Appear");
    LayoutActor::appear();
    setActionFrameRate(this, mFrames > 0 ? getActionFrameMax(this, nullptr) / mFrames : 1.0f);
    setNerve(this, &NrvWipeSimpleClose);
}

/**
 * Starts closing the wipe unless it is already closing or closed.
 * @param frames close duration in frames, or a non-positive value for the action's own length
 */
void WipeSimple::tryStartClose(s32 frames) {
    if (isAlive() && (isNerve(this, &NrvWipeSimpleClose) || isNerve(this, &NrvWipeSimpleCloseEnd))) {
        return;
    }

    startClose(frames);
}

/**
 * Makes the wipe appear already closed.
 */
void WipeSimple::startCloseEnd() {
    LayoutActor::appear();
    setNerve(this, &NrvWipeSimpleCloseEnd);
}

/**
 * Starts opening the wipe.
 * @param frames open duration in frames, or a non-positive value for the action's own length
 */
void WipeSimple::startOpen(s32 frames) {
    mFrames = frames;
    startAction(this, "End");
    setNerve(this, &NrvWipeSimpleOpen);
}

/**
 * Starts opening the wipe after a delay.
 * @param delay delay in frames, or a negative value to open immediately
 * @param frames open duration in frames, or a non-positive value for the action's own length
 */
void WipeSimple::startOpenDelay(s32 delay, s32 frames) {
    mDelay = delay;
    mFrames = frames;

    if (delay >= 0) {
        setNerve(this, &NrvWipeSimpleDelayOpen);
        return;
    }

    startOpen(frames);
}

/**
 * Starts opening the wipe unless it is already opening.
 * @param frames open duration in frames, or a non-positive value for the action's own length
 */
void WipeSimple::tryStartOpen(s32 frames) {
    if (!isAlive() || !isNerve(this, &NrvWipeSimpleOpen)) {
        startOpen(frames);
    }
}

/**
 * Checks whether the wipe is fully closed.
 * @return whether the wipe is closed
 */
bool WipeSimple::isCloseEnd() const {
    return isNerve(this, &NrvWipeSimpleCloseEnd);
}

/**
 * Waits for the close action to end.
 */
void WipeSimple::exeClose() {
    if (!isFirstStep(this) && isActionEnd(this)) {
        setNerve(this, &NrvWipeSimpleCloseEnd);
    }
}

/**
 * Plays the wait action while closed.
 */
void WipeSimple::exeCloseEnd() {
    if (isFirstStep(this)) {
        startAction(this, "Wait");
    }
}

/**
 * Opens the wipe and kills it once the open action has ended.
 */
void WipeSimple::exeOpen() {
    if (isFirstStep(this)) {
        setActionFrameRate(this, mFrames > 0 ? getActionFrameMax(this, nullptr) / mFrames : 1.0f);
    }

    if (isActionEnd(this)) {
        kill();
    }
}

/**
 * Waits for the delay, then opens the wipe and kills it once the open action has ended.
 */
void WipeSimple::exeDelayOpen() {
    if (isLessStep(this, mDelay)) {
        return;
    }

    if (isStep(this, mDelay)) {
        startAction(this, "End");
        setActionFrameRate(this, mFrames > 0 ? getActionFrameMax(this, nullptr) / mFrames : 1.0f);
    }

    if (isActionEnd(this)) {
        kill();
    }
}

/**
 * Returns the duration of the wipe.
 * @return duration in frames
 */
s32 WipeSimple::getWipeFrameNum() const {
    if (mFrames > 0) {
        return mFrames;
    }

    return getActionFrameMax(this, nullptr);
}

/**
 * Makes the wipe appear.
 */
void WipeSimple::appear() {
    LayoutActor::appear();
}
}  // namespace al
