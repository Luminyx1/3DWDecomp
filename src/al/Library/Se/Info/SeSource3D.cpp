#include "Library/Se/Info/SeSource.hpp"

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"
#include "Library/Se/Info/SeadAudioActorWrapper.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"
#include "Project/Audio/System/AudioConstMultiPlatformCommon.hpp"
#include "Project/Audio/System/SeadAudio3DMgr.hpp"

namespace {
const f32 cFarDistanceSq = al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_FAR_DISTANCE *
                           al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_FAR_DISTANCE;
const f32 cPriorityRate =
    al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX / cFarDistanceSq;
}  // namespace

namespace al {
/**
 * Constructs the 3D SE source base.
 * @param rName Source name.
 * @param pPose Source pose.
 * @param pInfo Audio system information.
 */
SeSource3D::SeSource3D(const sead::SafeString& rName, SeSourcePose3D* pPose, AudioSystemInfo* pInfo)
    : SeSource(rName), mPose(pPose), mInfo(pInfo) {}

/**
 * Creates the 3D actor and places it at the initial position.
 */
void SeSource3D::init() {
    mActor = new (16) SeadAudio3DActorWrapper();
    mActor->initialize(mInfo->mAudio3DMgr);
    calcPositionInitialize();
    syncPosition();
}

/**
 * Moves the 3D actor to the position nearest to the listener and updates the priority.
 */
void SeSource3D::syncPosition() {
    calcPositionDynamic();
    const sead::Vector3f& listenerPos = mInfo->mAudio3DMgr->getListenerPosition();
    const sead::Vector3f* pos = calcPosition(listenerPos);
    mActor->setPosition(*pos);
    f32 distanceSq = (*pos - listenerPos).squaredLength();
    if (cFarDistanceSq < distanceSq) {
        mPriority = -1;
    } else {
        mPriority = private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX -
                    static_cast<s32>(distanceSq * cPriorityRate);
    }
}

/**
 * Updates the source position.
 */
void SeSource3D::update() {
    syncPosition();
}

/**
 * Starts a sound on the 3D actor.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @return True if the sound started.
 */
bool SeSource3D::startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) {
    return mActor->startSoundWithInfo(pHandle, soundId, pStartInfo, false);
}

/**
 * Checks whether the 3D actor plays any sound.
 * @return True if a sound is playing.
 */
bool SeSource3D::isPlayingSound() const {
    return mActor->isPlayingSound();
}

/**
 * Gets the 3D actor position.
 * @return Actor position.
 */
const sead::Vector3f* SeSource3D::getPosition() const {
    return mActor->getPosition();
}

/**
 * Resets the 3D actor velocity.
 */
void SeSource3D::resetVelocity() {
    mActor->resetVelocity();
}
}  // namespace al
