#include "audio/seadAudio3DActor.h"

#include <nn/util/util_VectorApi.h>

#include "audio/seadAudio3DMgrNin.h"
#include "audio/seadAudioMgr.h"
#include "audio/seadAudioPlayerNin.h"
#include "audio/seadAudioResetter.h"
#include "audio/seadAudioSystemNin.h"
#include "audio/seadSoundHandle.h"

namespace sead {
namespace {
bool convertStartResult(nn::atk::SoundStartable::StartResult result, AudioStartResult* pResult) {
    if (pResult) {
        s32 code = result.GetCode();
        *pResult = static_cast<AudioStartResult>(code < cAudioStartResult_Unknown ?
                                                     code :
                                                     cAudioStartResult_Unknown);
    }

    return result.IsSuccess();
}
}  // namespace

/**
 * Constructs a 3D actor.
 */
Audio3DActorNin::Audio3DActorNin() = default;

/**
 * Attaches the actor to the audio player and a 3D manager.
 * @param pMgr 3D manager.
 */
void Audio3DActorNin::initialize(Audio3DMgr* pMgr) {
    AudioPlayerNin* player = DynamicCast<AudioPlayerNin>(AudioMgr::instance()->getPlayer());
    Audio3DMgrNin* mgr = DynamicCast<Audio3DMgrNin>(pMgr);
    Sound3DActor::Initialize(player, mgr->getSound3DManager());
}

/**
 * Detaches the actor.
 */
void Audio3DActorNin::finalize() {
    Sound3DActor::Finalize();
}

/**
 * Sets the actor position.
 * @param rPosition Position.
 */
void Audio3DActorNin::setPosition(const Vector3f& rPosition) {
    nn::util::Vector3fType position;
    nn::util::VectorSet(&position, rPosition.x, rPosition.y, rPosition.z);
    SetPosition(position);
}

/**
 * Resets the actor position so that the next position is not used for velocity.
 */
void Audio3DActorNin::resetPosition() {
    ResetPosition();
}

/**
 * Starts a sound on the actor.
 * @param pHandle Handle to attach the sound to.
 * @param soundId Sound ID.
 * @param pResult Receives the start result, may be nullptr.
 * @return True if the sound was started.
 */
bool Audio3DActorNin::startSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) {
    return convertStartResult(StartSound(pHandle, soundId), pResult);
}

/**
 * Starts a sound on the actor.
 * @param pHandle Handle to attach the sound to.
 * @param pSoundName Sound label.
 * @param pResult Receives the start result, may be nullptr.
 * @return True if the sound was started.
 */
bool Audio3DActorNin::startSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) {
    return convertStartResult(StartSound(pHandle, pSoundName), pResult);
}

/**
 * Holds a sound on the actor.
 * @param pHandle Handle to attach the sound to.
 * @param soundId Sound ID.
 * @param pResult Receives the start result, may be nullptr.
 * @return True if the sound is playing.
 */
bool Audio3DActorNin::holdSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) {
    return convertStartResult(HoldSound(pHandle, soundId), pResult);
}

/**
 * Holds a sound on the actor.
 * @param pHandle Handle to attach the sound to.
 * @param pSoundName Sound label.
 * @param pResult Receives the start result, may be nullptr.
 * @return True if the sound is playing.
 */
bool Audio3DActorNin::holdSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) {
    return convertStartResult(HoldSound(pHandle, pSoundName), pResult);
}

/**
 * Sets the actor velocity.
 * @param rVelocity Velocity.
 */
void Audio3DActorNin::setVelocity(const Vector3f& rVelocity) {
    nn::util::Vector3fType velocity;
    nn::util::VectorSet(&velocity, rVelocity.x, rVelocity.y, rVelocity.z);
    SetVelocity(velocity);
}

/**
 * Gets the actor velocity.
 * @return Velocity.
 */
Vector3f Audio3DActorNin::getVelocity() const {
    const nn::util::Vector3fType& velocity = GetVelocity();
    return Vector3f(nn::util::VectorGetX(velocity), nn::util::VectorGetY(velocity),
                    nn::util::VectorGetZ(velocity));
}

/**
 * Stops every sound of the actor.
 * @param fadeFrames Fade-out length in frames.
 */
void Audio3DActorNin::stopAllSound(s32 fadeFrames) {
    StopAllSound(fadeFrames);
}

/**
 * Pauses or resumes every sound of the actor.
 * @param pause True to pause, false to resume.
 * @param fadeFrames Fade length in frames.
 */
void Audio3DActorNin::pauseAllSound(bool pause, s32 fadeFrames) {
    PauseAllSound(pause, fadeFrames);
}

/**
 * Sets up a sound unless starting is disabled or the audio is resetting.
 * @param pHandle Handle to attach the sound to.
 * @param soundId Sound ID.
 * @param pStartInfo Start parameters.
 * @param pSetupArg Setup argument.
 * @return Start result.
 */
nn::atk::SoundStartable::StartResult Audio3DActorNin::SetupSound(nn::atk::SoundHandle* pHandle, u32 soundId,
                                                                const StartInfo* pStartInfo,
                                                                void* pSetupArg) {
    DynamicCast<AudioSystemNin>(AudioMgr::instance()->getAudioSystem());
    if (mIsStartDisabled || AudioMgr::instance()->getResetter()->isResetting()) {
        return StartResult(StartResult::ResultCode_ErrorUser);
    }

    return Sound3DActor::SetupSound(pHandle, soundId, pStartInfo, pSetupArg);
}
}  // namespace sead
