#include "Library/Se/Info/SeSource.hpp"

#include <attributes.h>
#include <audio/seadSoundHandle.h>
#include <nn/util/util_VectorApi.h>

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"
#include "Library/Se/Info/SeadAudioActorWrapper.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"
#include "Project/Audio/System/AudioConstMultiPlatformCommon.hpp"
#include "Project/Audio/System/SeadAudio3DMgr.hpp"

namespace {
/**
 * @brief Projects a planar position onto a disk centered at the origin.
 * @param rPosition Plane coordinates to clamp in place.
 * @param rpRadius Reference to a non-null disk-radius pointer, read after the length calculation.
 */
inline void clampToDisk(sead::Vector2f& rPosition, const f32* const& rpRadius) {
    f32 length = rPosition.length();
    if (al::isNearZero(length, 0.001f)) {
        rPosition.set(0.0f, -*rpRadius);
    } else if (length >= *rpRadius) {
        rPosition *= *rpRadius / length;
    }
}
} // namespace

namespace al {
/**
 * @brief Constructs a circle source.
 * @param pPose Non-null source pose, which must outlive this source.
 * @param pRadius Non-null radius pointer, which must remain valid while the source is used.
 * @param pInfo Non-null audio system information used to access the 3D manager.
 * @param isVertical Whether the circle is in the XY plane instead of the XZ plane.
 */
SeSource3DCircle::SeSource3DCircle(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo,
                                   bool isVertical)
    : SeSource3D("３Ｄ平面円音源", pPose, pInfo), mMtxPose(pPose), mRadius(pRadius), mIsVertical(isVertical) {
    mInvMtx.makeIdentity();
}

/**
 * @brief Does nothing.
 */
void SeSource3DCircle::calcPositionInitialize() {}

/**
 * @brief Updates the pose and the inverse of its matrix.
 */
void SeSource3DCircle::calcPositionDynamic() {
    mMtxPose->update();
    mInvMtx.setInverse(mMtxPose->get3DMtx());
}

/**
 * @brief Finds the nearest point on the source disk to the pListener.
 * @param rListenerPos Listener position in world coordinates.
 * @return Source-owned world-space position on the disk.
 */
const sead::Vector3f* SeSource3DCircle::calcPosition(const sead::Vector3f& rListenerPos) {
    const sead::Matrix34f matrix = mInvMtx;
    sead::Vector3f localPos;
    localPos.setMul(matrix, rListenerPos);

    if (mIsVertical) {
        sead::Vector2f planePos(localPos.x, localPos.y);
        clampToDisk(planePos, mRadius);

        mPos.set(planePos.x, planePos.y, 0.0f);
    } else {
        sead::Vector2f planePos(localPos.x, localPos.z);
        clampToDisk(planePos, mRadius);

        mPos.set(planePos.x, 0.0f, planePos.y);
    }

    mPos.setMul(mMtxPose->get3DMtx(), mPos);
    return &mPos;
}
} // namespace al

namespace {
const f32 cFarDistanceSq =
    al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_FAR_DISTANCE *
    al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_FAR_DISTANCE;
const f32 cPriorityRate =
    al::private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX / cFarDistanceSq;
} // namespace

namespace al {
/**
 * @brief Constructs the 3D SE source base.
 * @param rName Source name.
 * @param pPose Non-null source pose, which must outlive this source.
 * @param pInfo Non-null audio system information used to access the 3D manager.
 */
NOINLINE SeSource3D::SeSource3D(const sead::SafeString& rName, SeSourcePose3D* pPose, AudioSystemInfo* pInfo)
    : SeSource(rName), mPose(pPose), mInfo(pInfo) {}

/**
 * @brief Creates the 3D actor and places it at the initial position.
 */
void SeSource3D::init() {
    mActor = new (16) SeadAudio3DActorWrapper();
    mActor->initialize(mInfo->mAudio3DMgr);
    calcPositionInitialize();
    syncPosition();
}

/**
 * @brief Moves the 3D actor to the position nearest to the pListener and updates the priority.
 */
void SeSource3D::syncPosition() {
    calcPositionDynamic();
    const sead::Vector3f& rListenerPos = mInfo->mAudio3DMgr->getListenerPosition();
    const sead::Vector3f* pPosition = calcPosition(rListenerPos);
    mActor->setPosition(*pPosition);
    f32 distanceSq = (*pPosition - rListenerPos).squaredLength();

    if (cFarDistanceSq < distanceSq) {
        mPriority = -1;
    } else {
        mPriority = private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX -
                    static_cast<s32>(distanceSq * cPriorityRate);
    }
}

/**
 * @brief Updates the source position.
 */
void SeSource3D::update() { syncPosition(); }

/**
 * @brief Starts a sound on the 3D actor.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @return True if the sound started.
 */
bool SeSource3D::startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) {
    return mActor->startSoundWithInfo(pHandle, soundId, pStartInfo, false);
}

/**
 * @brief Checks whether the 3D actor plays any sound.
 * @return True if a sound is playing.
 */
bool SeSource3D::isPlayingSound() const { return mActor->isPlayingSound(); }

/**
 * @brief Gets the 3D actor position.
 * @return Actor position.
 */
const sead::Vector3f* SeSource3D::getPosition() const { return mActor->getPosition(); }

/**
 * @brief Resets the 3D actor velocity.
 */
void SeSource3D::resetVelocity() { mActor->resetVelocity(); }
} // namespace al

namespace al {
/**
 * @brief Starts a sound with start information.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @param isHold Unused; this wrapper always starts the sound.
 * @return True if the sound started.
 */
bool SeadAudio3DActorWrapper::startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId,
                                                 const SoundStartInfo* pStartInfo, bool isHold) {
    return StartSound(pHandle, soundId,
                      reinterpret_cast<const nn::atk::SoundStartable::StartInfo*>(pStartInfo))
        .IsSuccess();
}

/**
 * @brief Gets the actor position.
 * @return Actor position.
 */
NOINLINE const sead::Vector3f* SeadAudio3DActorWrapper::getPosition() const {
    return reinterpret_cast<const sead::Vector3f*>(&GetPosition());
}

/**
 * @brief Resets the actor velocity.
 */
NOINLINE void SeadAudio3DActorWrapper::resetVelocity() {
    nn::util::Vector3fType zero;
    nn::util::VectorSet(&zero, 0.0f, 0.0f, 0.0f);
    SetVelocity(zero);
}

/**
 * @brief Checks whether the actor plays any sound.
 * @return True if the actor plays a sound.
 */
NOINLINE bool SeadAudio3DActorWrapper::isPlayingSound() const {
    for (s32 i = 0; i < 4; i++) {
        if (GetPlayingSoundCount(i) > 0) {
            return true;
        }
    }

    return false;
}
} // namespace al

namespace al {
/**
 * @brief Constructs the SE source base.
 * @param rName Source name.
 */
NOINLINE SeSource::SeSource(const sead::SafeString& rName) : mName(rName) {}

/**
 * @brief Gets the position of the default pListener.
 * @return Listener position.
 */
NOINLINE const sead::Vector3f& SeadAudio3DMgr::getListenerPosition() const {
    const nn::atk::Sound3DListener* pListener = getDefaultListener();
    return reinterpret_cast<const sead::Vector3f&>(pListener->GetPosition());
}
} // namespace al
