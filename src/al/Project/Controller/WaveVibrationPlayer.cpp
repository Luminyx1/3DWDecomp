#include <al/Project/Controller/WaveVibrationPlayer.hpp>

namespace al {
int compareStringIgnoreCase(const char*, const char*);

namespace {
/**
 * Sets both bands of one output's modulation.
 * @param pPlayer Source vibration player.
 * @param pConnection Output connection.
 * @param volume Amplitude multiplier.
 * @param pitch Frequency multiplier.
 */
void setModulation(nn::hid::VibrationNodeConnection* pConnection,
                   nn::hid::VibrationPlayer* pPlayer, f32 volume, f32 pitch)
{
    nn::hid::VibrationModulation modulation{volume, pitch, volume, pitch};
    pPlayer->SetModulationTo(pConnection->GetDestination(), modulation);
}
/**
 * Replaces the amplitude multipliers while preserving the other components.
 * @param pPlayer Source vibration player.
 * @param pConnection Output connection.
 * @param value Multiplier for both bands.
 */
void setVolume(nn::hid::VibrationNodeConnection* pConnection,
                nn::hid::VibrationPlayer* pPlayer, f32 value)
{
    auto modulation = pConnection->GetModulation();
    modulation.amplitudeLow = value;
    modulation.amplitudeHigh = value;
    pPlayer->SetModulationTo(pConnection->GetDestination(), modulation);
}
/**
 * Replaces the frequency multipliers while preserving the other components.
 * @param pPlayer Source vibration player.
 * @param pConnection Output connection.
 * @param value Multiplier for both bands.
 */
void setPitch(nn::hid::VibrationNodeConnection* pConnection,
                nn::hid::VibrationPlayer* pPlayer, f32 value)
{
    auto modulation = pConnection->GetModulation();
    modulation.frequencyLow = value;
    modulation.frequencyHigh = value;
    pPlayer->SetModulationTo(pConnection->GetDestination(), modulation);
}
}  // namespace

/**
 * Orders vibration resources by their case-insensitive names.
 * @param rOther Resource to compare against.
 * @return Whether this resource sorts before the other resource.
 */
bool WaveVibrationData::operator<(const WaveVibrationData& rOther) const
{
    return compareStringIgnoreCase(rOther.name, name) > 0;
}

/**
 * Starts one-shot vibration with playback bookkeeping.
 * @param pData Vibration resource, or nullptr to clear the duration.
 * @param leftVolume Amplitude multiplier for the left output.
 * @param rightVolume Amplitude multiplier for the right output.
 * @param leftPitch Frequency multiplier for the left output.
 * @param rightPitch Frequency multiplier for the right output.
 * @param priority Priority associated with this playback request.
 * @param duration Minimum number of frames reported as playing; negative disables it.
 * @param syncToFrames Whether update synchronizes the sample position to frame count.
 */
void WaveVibrationPlayer::startOneTime(const WaveVibrationData* pData, f32 leftVolume,
    f32 rightVolume, f32 leftPitch, f32 rightPitch, s32 priority, s32 duration, bool syncToFrames)
{
    mPriority = priority;
    mDuration = duration;
    mSyncToFrames = syncToFrames;
    mPaused = false;
    mPauseLocked = false;
    start(pData, leftVolume, rightVolume, leftPitch, rightPitch, false);
}

/**
 * Starts looping vibration with playback bookkeeping.
 * @param pData Vibration resource, or nullptr to clear the duration.
 * @param leftVolume Amplitude multiplier for the left output.
 * @param rightVolume Amplitude multiplier for the right output.
 * @param leftPitch Frequency multiplier for the left output.
 * @param rightPitch Frequency multiplier for the right output.
 * @param priority Priority associated with this playback request.
 * @param duration Minimum number of frames reported as playing; negative disables it.
 * @param syncToFrames Whether update synchronizes the sample position to frame count.
 */
void WaveVibrationPlayer::startLoop(const WaveVibrationData* pData, f32 leftVolume,
    f32 rightVolume, f32 leftPitch, f32 rightPitch, s32 priority, s32 duration, bool syncToFrames)
{
    mPriority = priority;
    mDuration = duration;
    mSyncToFrames = syncToFrames;
    mPaused = false;
    mPauseLocked = false;
    start(pData, leftVolume, rightVolume, leftPitch, rightPitch, true);
}

/**
 * Configures output modulation, loads the resource and starts playback.
 * @param pData Resource to play, or nullptr to clear the duration.
 * @param leftVolume Left amplitude multiplier.
 * @param rightVolume Right amplitude multiplier.
 * @param leftPitch Left frequency multiplier.
 * @param rightPitch Right frequency multiplier.
 * @param loop Whether playback loops.
 */
void WaveVibrationPlayer::start(const WaveVibrationData* pData, f32 leftVolume,
    f32 rightVolume, f32 leftPitch, f32 rightPitch, bool loop)
{
    setModulation(mLeft, mPlayer, leftVolume, leftPitch);
    setModulation(mRight, mPlayer, rightVolume, rightPitch);
    mFrame = 0;
    mElapsed = 0;
    if (pData) {
        mPlayer->Load(pData->data, pData->size);
        mPlayer->Play();
        mPlayer->SetLoop(loop);
        mData = pData;
    } else {
        mDuration = 0;
    }
}

/** Stops playback and clears the active resource, pause state and elapsed frames. */
void WaveVibrationPlayer::stop()
{
    mPlayer->Stop();
    mData = nullptr;
    mFrame = 0;
    mPaused = false;
    mPauseLocked = false;
    mDuration = 0;
    mElapsed = 0;
}

/**
 * Pauses playback unless another locked pause already owns it.
 * @param locked Whether this pause requires a matching locked resume.
 */
void WaveVibrationPlayer::pause(bool locked)
{
    if (mPauseLocked)
        return;
    mPauseLocked = locked;
    mPaused = true;
    mPlayer->Stop();
}

/**
 * Resumes playback when the pause-lock category matches.
 * @param locked Whether to resume a locked pause.
 */
void WaveVibrationPlayer::endPause(bool locked)
{
    if (locked) {
        if (!mPauseLocked)
            return;
    } else if (mPauseLocked) {
        return;
    }
    mPaused = false;
    mPauseLocked = false;
    mPlayer->Play();
}

/** @return Whether the minimum duration or the underlying player is still active. */
bool WaveVibrationPlayer::isPlaying() const
{
    if (mDuration >= 0 && mElapsed < mDuration)
        return true;
    return mPlayer->IsPlaying();
}

/** @return Whether the underlying player exists and is configured to loop. */
bool WaveVibrationPlayer::isLoop() const
{
    return mPlayer && mPlayer->IsLoop();
}

/**
 * Changes both outputs' amplitude multipliers.
 * @param left Multiplier for both bands of the left output.
 * @param right Multiplier for both bands of the right output.
 */
void WaveVibrationPlayer::changeVolume(f32 left, f32 right)
{
    if (!isPlaying())
        return;
    setVolume(mLeft, mPlayer, left);
    setVolume(mRight, mPlayer, right);
}

/**
 * Changes both outputs' frequency multipliers.
 * @param left Multiplier for both bands of the left output.
 * @param right Multiplier for both bands of the right output.
 */
void WaveVibrationPlayer::changePitch(f32 left, f32 right)
{
    if (!isPlaying() && (!mPaused || mPauseLocked))
        return;
    setPitch(mLeft, mPlayer, left);
    setPitch(mRight, mPlayer, right);
}

/**
 * Changes amplitude and frequency multipliers for both outputs.
 * @param leftVolume Left amplitude multiplier.
 * @param rightVolume Right amplitude multiplier.
 * @param leftPitch Left frequency multiplier.
 * @param rightPitch Right frequency multiplier.
 */
void WaveVibrationPlayer::changeVolumeAndPitch(f32 leftVolume, f32 rightVolume,
                                             f32 leftPitch, f32 rightPitch)
{
    if (!isPlaying() && (!mPaused || mPauseLocked))
        return;
    setModulation(mLeft, mPlayer, leftVolume, leftPitch);
    setModulation(mRight, mPlayer, rightVolume, rightPitch);
}
}  // namespace al
