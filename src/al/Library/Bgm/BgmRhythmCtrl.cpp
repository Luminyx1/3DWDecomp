#include "Library/Bgm/BgmRhythmCtrl.hpp"

#include "Library/Bgm/BgmLine.hpp"
#include "Library/Bgm/BgmRhythmDetector.hpp"

namespace {
/**
 * Gets the rhythm detector of the active BGM line.
 * @param pActiveBgmLine Active BGM line provider.
 * @return Rhythm detector.
 */
const al::BgmRhythmDetector* getDetector(const al::IUseActiveBgmLine* pActiveBgmLine) {
    const al::BgmLine* line = pActiveBgmLine->getActiveBgmLine();
    line->isEnableRhythmDetection();
    return line->getRhythmDetector();
}
}  // namespace

namespace al {
/**
 * Constructs a rhythm controller.
 * @param frameRate Frame rate.
 */
BgmRhythmCtrl::BgmRhythmCtrl(f32 frameRate) : mFrameRate(frameRate) {}

/**
 * Sets the provider of the active BGM line.
 * @param pActiveBgmLine Active BGM line provider.
 */
void BgmRhythmCtrl::init(IUseActiveBgmLine* pActiveBgmLine) {
    mActiveBgmLine = pActiveBgmLine;
}

/**
 * Does nothing.
 */
void BgmRhythmCtrl::update() {}

/**
 * Gets the BPM of the active line scaled by the frame rate.
 * @return Current BPM.
 */
f32 BgmRhythmCtrl::getCurrentBpm() const {
    return mActiveBgmLine->getActiveBgmLine()->getCurBpm() * mFrameRate;
}

/**
 * Checks whether the active line provides rhythm information.
 * @return True if the rhythm animation is enabled.
 */
bool BgmRhythmCtrl::isEnableRhythmAnim() const {
    const BgmLine* line = mActiveBgmLine->getActiveBgmLine();
    if (line == nullptr) {
        return false;
    }
    if (!line->isEnableRhythmDetection()) {
        return false;
    }
    return line->getRhythmDetector() != nullptr;
}

/**
 * Checks whether the BGM has been restarted.
 * @return True if the BGM has been restarted.
 */
bool BgmRhythmCtrl::isTriggerRestartBgm() const {
    return getDetector(mActiveBgmLine)->isTriggerRestartBgm();
}

/**
 * Checks whether a beat is triggered.
 * @param beat Beat.
 * @return True if the beat is triggered.
 */
bool BgmRhythmCtrl::isTriggerBeat(s32 beat) const {
    const BgmLine* line = mActiveBgmLine->getActiveBgmLine();
    const BgmRhythmDetector* detector =
        line != nullptr && line->isEnableRhythmDetection() ? line->getRhythmDetector() : nullptr;
    return detector->isTriggerBeat(beat);
}

/**
 * Checks whether a beat for animation is triggered.
 * @param beat Beat.
 * @return True if the beat is triggered.
 */
bool BgmRhythmCtrl::isTriggerBeatForAnime(s32 beat) const {
    const BgmLine* line = mActiveBgmLine->getActiveBgmLine();
    const BgmRhythmDetector* detector =
        line != nullptr && line->isEnableRhythmDetection() ? line->getRhythmDetector() : nullptr;
    return detector->isTriggerBeatForAnime(beat);
}

/**
 * Checks whether a rhythm is triggered.
 * @return True if a rhythm is triggered.
 */
bool BgmRhythmCtrl::isTriggerRhythm() const {
    return getDetector(mActiveBgmLine)->isTriggerRhythm();
}

/**
 * Checks whether the animation type changed.
 * @return True if the animation type changed.
 */
bool BgmRhythmCtrl::isTriggerAnimChange() const {
    return getDetector(mActiveBgmLine)->isTriggerAnimChange();
}

/**
 * Gets the current animation type.
 * @return Animation type.
 */
s32 BgmRhythmCtrl::getAnimType() const {
    return getDetector(mActiveBgmLine)->getAnimType();
}

/**
 * Gets the current animation frame.
 * @return Animation frame.
 */
f32 BgmRhythmCtrl::getAnimFrame() const {
    return getDetector(mActiveBgmLine)->getAnimFrame();
}

/**
 * Gets the progress rate in the current beat.
 * @return Beat rate.
 */
f32 BgmRhythmCtrl::getBeatRate() const {
    return getDetector(mActiveBgmLine)->getBeatRate();
}

/**
 * Gets the progress rate in the current beat for animation.
 * @return Beat rate for animation.
 */
f32 BgmRhythmCtrl::getBeatRateForAnime() const {
    return getDetector(mActiveBgmLine)->getBeatRateForAnime();
}

/**
 * Gets the current chord information.
 * @return Chord information.
 */
const void* BgmRhythmCtrl::getChordInfoCurrent() const {
    return getDetector(mActiveBgmLine)->getChordInfoCurrent();
}

/**
 * Gets the current beat.
 * @return Current beat.
 */
f32 BgmRhythmCtrl::getCurBeat() const {
    return getDetector(mActiveBgmLine)->getCurBeat();
}

/**
 * Gets the number of beats per frame.
 * @return Beats per frame.
 */
f32 BgmRhythmCtrl::getBeatPerFrame() const {
    return getDetector(mActiveBgmLine)->getBeatPerFrame();
}

/**
 * Gets the frame rate of the rhythm detector.
 * @return Frame rate.
 */
f32 BgmRhythmCtrl::getFrameRate() const {
    return getDetector(mActiveBgmLine)->getFrameRate();
}
}  // namespace al
