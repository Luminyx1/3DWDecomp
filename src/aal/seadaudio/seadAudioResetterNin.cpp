#include "audio/seadAudioResetter.h"

#include <nn/atk/atk_HardwareManager.h>

#include "audio/seadAudioMgr.h"
#include "audio/seadAudioPlayerNin.h"

namespace sead {
/**
 * Constructs a resetter that fades the master volume out during resets.
 */
AudioResetterNin::AudioResetterNin() = default;

/**
 * Initializes the resetter.
 * @param rMgr Audio manager.
 */
void AudioResetterNin::initialize(AudioMgr& rMgr) {
    AudioResetter::initialize(rMgr);
}

/**
 * Advances the reset and shutdown states once the master volume faded out.
 */
void AudioResetterNin::calc() {
    f32 volume = nn::atk::detail::driver::HardwareManager::GetInstance().GetMasterVolume();

    if (mShutdownState == cState_Running) {
        if (volume == 0.0f) {
            mShutdownState = cState_Done;
            return;
        }
    }

    if (mShutdownState == cState_Done) {
        return;
    }

    if (mResetState == cState_Running) {
        if (volume == 0.0f) {
            DynamicCast<AudioPlayerNin>(mAudioMgr->getPlayer())->stopAll(0);
            mResetState = cState_Done;
        }
    }
}

/**
 * Starts a reset by fading out the master volume.
 * @param fadeFrames Fade-out length in frames.
 */
void AudioResetterNin::reset(s32 fadeFrames) {
    if (mShutdownState != cState_None || mResetState != cState_None) {
        return;
    }

    mMasterVolume = nn::atk::detail::driver::HardwareManager::GetInstance().GetMasterVolume();
    nn::atk::detail::driver::HardwareManager::GetInstance().SetMasterVolume(0.0f, fadeFrames);
    AudioResetter::reset(fadeFrames);
    mResetState = cState_Running;
}

/**
 * Checks whether a reset is in progress.
 * @return True while resetting.
 */
bool AudioResetterNin::isResetting() const {
    if (mResetState != cState_None) {
        return true;
    }

    return AudioResetter::isResetting();
}

/**
 * Checks whether the reset finished.
 * @return True once the reset is done.
 */
bool AudioResetterNin::isResetDone() const {
    if (!AudioResetter::isResetDone()) {
        return false;
    }

    return mResetState == cState_Done;
}

/**
 * Restores the master volume and resumes all sounds after a reset.
 */
void AudioResetterNin::recoverReset() {
    if (mShutdownState != cState_None) {
        return;
    }

    f32 volume = mMasterVolume;
    nn::atk::detail::driver::HardwareManager::GetInstance().SetMasterVolume(volume, 0);
    AudioResetter::recoverReset();
    DynamicCast<AudioPlayerNin>(mAudioMgr->getPlayer())->unpauseAll(0);
    mResetState = cState_None;
}

/**
 * Starts a shutdown by fading out the master volume.
 * @param fadeFrames Fade-out length in frames.
 */
void AudioResetterNin::shutdown(s32 fadeFrames) {
    if (mShutdownState != cState_None) {
        return;
    }

    nn::atk::detail::driver::HardwareManager::GetInstance().SetMasterVolume(0.0f, fadeFrames);
    AudioResetter::shutdown(fadeFrames);
    mShutdownState = cState_Running;
}

/**
 * Checks whether a shutdown is in progress.
 * @return True while shutting down.
 */
bool AudioResetterNin::isShuttingDown() const {
    if (mShutdownState != cState_None) {
        return true;
    }

    return AudioResetter::isShuttingDown();
}

/**
 * Checks whether the shutdown finished.
 * @return True once the shutdown is done.
 */
bool AudioResetterNin::isShutdownDone() const {
    if (!AudioResetter::isShutdownDone()) {
        return false;
    }

    return mShutdownState == cState_Done;
}
}  // namespace sead
