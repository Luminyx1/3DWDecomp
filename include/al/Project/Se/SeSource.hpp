#pragma once

#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class AcLSoundHandle;
class AudioSystemInfo;
class SeadAudio3DActorWrapper;
class SeSourcePose3D;
struct SoundStartInfo;

class SeSource {
public:
    SeSource(const sead::SafeString& rName);

    virtual void init() = 0;
    virtual void update() = 0;
    virtual bool startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pInfo) = 0;
    virtual bool isPlayingSound() const = 0;
    virtual s32 getPriority() const = 0;
    virtual void resetVelocity() = 0;
    virtual bool isCalc3D() const;
    virtual const sead::Vector3f& getPosition() const = 0;
    virtual ~SeSource() {}

    sead::SafeString mName;  // _8
    f32 _18;
};

class SeSource3D : public SeSource {
public:
    SeSource3D(const sead::SafeString& rName, SeSourcePose3D* pPose, AudioSystemInfo* pInfo);

    void init() override;
    void update() override;
    bool startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pInfo) override;
    bool isPlayingSound() const override;
    s32 getPriority() const override;
    void resetVelocity() override;
    bool isCalc3D() const override;
    const sead::Vector3f& getPosition() const override;
    ~SeSource3D() override {}

    virtual void calcPositionInitialize() = 0;
    virtual void calcPositionDynamic() = 0;
    virtual const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) = 0;

    void syncPosition();

    SeadAudio3DActorWrapper* mActor;  // _20
    SeSourcePose3D* mPose;            // _28
    s32 mPriority;                    // _30
    AudioSystemInfo* mAudioSystemInfo;  // _38
};
}  // namespace al
