#pragma once

#include <basis/seadTypes.h>
#include <container/seadListImpl.h>
#include <math/seadVector.h>

namespace al {
class AcLSoundHandle;
class AudioMixVolume;
class SePlayParamList;
class SeResourceSpecificInfo;
class SeSource;

class SeRequest {
  public:
    SeRequest();
    void clear();
    void stop(s32 fadeFrames);
    bool stopIfEqualIdOrOriginalId(u32 soundId, SeSource* pSource, s32 fadeFrames);
    u32 getOriginalId() const;
    SePlayParamList* setNew(u32 soundId, SeSource* pSource, bool isLoop, bool isHold,
                            const SeResourceSpecificInfo* pSpecificInfo);
    void pause();
    bool verifyData() const;
    void tryPauseBySystem(bool isPause, u32 fadeFrames);
    void playStart(bool isAfterGoal);
    SePlayParamList* extendHold();
    void setStateWaitForStart();
    void releaseHandle();
    bool isWaveSound() const;
    bool isFinishedPlaying() const;
    void pauseByDistanceSe();
    void unpauseByDistanceSe();
    bool isAttachedSound() const;
    void updateLifeTime();
    void updateSeSource();
    void updatePlayCount();
    void applyParamToRequest(bool isAfterGoal, f32 volume, f32 distance);
    bool calcDistance(sead::Vector3f& rListenerPos, s32 order);
    const sead::Vector3f* getPosition() const;
    void setMulParamVolume(f32 volume);
    void setParamLpfFreq(f32 freq);
    void applyVolume(bool isAfterGoal, f32 volume);
    static s32 compareSeRequestByDistance(const SeRequest* pA, const SeRequest* pB);

    enum PlaySelection { Undecided = 0, Play = 1, Skip = 2, Delay = 3 };
    /** @brief Gets the request's playback state. @return Empty (0), new (1), playing (2), paused (3), or
     * waiting (4). */
    s32 getState() const { return mState; }
    /** @brief Gets the selection made during this keeper update. @return Current playback selection. */
    PlaySelection getPlaySelection() const { return static_cast<PlaySelection>(mPlayOrder); }
    /**
     * @brief Records the playback decision for this update.
     * @param selection Whether to
     * evaluate, play, skip, or delay the request.
     */
    void setPlaySelection(PlaySelection selection) { mPlayOrder = selection; }
    /** @brief Gets the elapsed playback counter. @return Number of non-paused playback updates. */
    s32 getPlayCount() const { return mPlayCount; }
    /** @brief Gets the listener distance including its sorting bias. @return Biased distance. */
    f32 getDistance() const { return mDistance; }
    /** @brief Tests whether distance currently prevents playback. @return Distance-pause flag. */
    bool isPausedByDistance() const { return mIsPausedByDistance; }
    /**
     * @brief Sets the delay multiplier assigned while limiting simultaneous triggers.
     * @param
     * multiplier Positive ordering index among requests queued for delayed playback.
     */
    void setDelayMultiplier(s32 multiplier) { mDelayMultiplier = multiplier; }

    /**
     * @brief Gets the sound-archive identifier of this request.
     * @return Current sound identifier.
     */
    u32 getSoundId() const { return mSoundId; }

    /**
     * @brief Gets the resource settings used by this sound request.
     * @return Resource-specific playback settings; required for a delayed request.
     */
    const SeResourceSpecificInfo* getSpecificInfo() const { return mSpecificInfo; }

    /**
     * @brief Gets the multiplier applied to the resource's delay in frames.
     * @return Delay multiplier assigned to this request.
     */
    s32 getDelayMultiplier() const { return mDelayMultiplier; }

    /** @brief Gets the source associated with this request. @return Source, or nullptr for an empty request.
     */
    SeSource* getSource() const { return mSource; }
    /** @brief Gets the request's volume controller. @return Allocated volume controller. */
    AudioMixVolume* getMixVolume() const { return mMixVolume; }
    /** @brief Tests whether this request retains looping playback. @return Stored loop flag. */
    bool isLoop() const { return mIsLoop; }
    /** @brief Gets the remaining hold lifetime. @return Remaining frames, or -1 for indefinite playback. */
    s32 getLifeTime() const { return mLifeTime; }

    /** @brief Gets the embedded list-node offset. @return Byte offset used by the active request list. */
    static s32 getNodeOffset() { return offsetof(SeRequest, mNode); }

  private:
    void stopAndClear(s32 fadeFrames);
    u32 calcOriginalId() const;
    void clearState();
    void pauseRequest();
    u32 mSoundId;
    const SeResourceSpecificInfo* mSpecificInfo;
    s32 mState;
    s32 mPlayOrder;
    s32 mPlayCount;
    f32 mDistance;
    s32 mLifeTime;
    bool mIsLoop;
    bool mIsPausedByDistance;
    AcLSoundHandle* mHandle;
    SeSource* mSource;
    SePlayParamList* mParamList;
    AudioMixVolume* mMixVolume;
    s32 mDelayMultiplier;
    f32 mVolume;
    sead::ListNode mNode;
};

static_assert(sizeof(SeRequest) == 0x60);
} // namespace al
