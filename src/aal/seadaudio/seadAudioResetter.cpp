#include "audio/seadAudioResetter.h"

#include "audio/seadAudioMgr.h"

namespace sead {
/**
 * Constructs a resetter that is not attached to an audio manager yet.
 */
AudioResetter::AudioResetter() = default;

/**
 * Attaches the resetter to an audio manager.
 * @param rMgr Audio manager.
 */
void AudioResetter::initialize(AudioMgr& rMgr) {
    mAudioMgr = &rMgr;
}

/**
 * Resets every audio subset.
 * @param fadeFrames Fade-out length in frames.
 */
void AudioResetter::reset(s32 fadeFrames) {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        it->reset(fadeFrames);
    }
}

/**
 * Checks whether any audio subset is still resetting.
 * @return True if a subset is resetting.
 */
bool AudioResetter::isResetting() const {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return false;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        if (it->isResetting()) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether every audio subset finished resetting.
 * @return True if all subsets are reset.
 */
bool AudioResetter::isResetDone() const {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return true;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        if (!it->isResetDone()) {
            return false;
        }
    }
    return true;
}

/**
 * Recovers every audio subset from a reset.
 */
void AudioResetter::recoverReset() {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        it->recoverReset();
    }
}

/**
 * Shuts down every audio subset.
 * @param fadeFrames Fade-out length in frames.
 */
void AudioResetter::shutdown(s32 fadeFrames) {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        it->shutdown(fadeFrames);
    }
}

/**
 * Checks whether any audio subset is still shutting down.
 * @return True if a subset is shutting down.
 */
bool AudioResetter::isShuttingDown() const {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return false;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        if (it->isShuttingDown()) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether every audio subset finished shutting down.
 * @return True if all subsets are shut down.
 */
bool AudioResetter::isShutdownDone() const {
    if (mAudioMgr->getSubsetList().isEmpty()) {
        return true;
    }
    for (auto it = mAudioMgr->getSubsetList().begin(); it != mAudioMgr->getSubsetList().end(); ++it) {
        if (!it->isShutdownDone()) {
            return false;
        }
    }
    return true;
}
}  // namespace sead
