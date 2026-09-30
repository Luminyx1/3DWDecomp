#pragma once

#include <audio/seadAudio3DActor.h>
#include <heap/seadDisposer.h>
#include <math/seadVector.h>
#include <nn/atk/atk_SoundActor.h>

namespace sead {
class SoundHandle;
}

namespace al {
class SeadAudioPlayer;
class SoundStartInfo;

class SeadAudio3DActorWrapper : public sead::Audio3DActorNin, public sead::IDisposer {
public:
    virtual bool startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo,
                                    bool isHold);

    const sead::Vector3f* getPosition() const;
    void resetVelocity();
    bool isPlayingSound() const;
};

static_assert(sizeof(SeadAudio3DActorWrapper) == 0x150);

class SeadAudioActorWrapper : public nn::atk::SoundActor, public sead::IDisposer {
public:
    ~SeadAudioActorWrapper() override;

    void stopAllSound(s32 fadeFrames);
    void initialize(SeadAudioPlayer* pPlayer);
    bool startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo, bool isHold);
    bool isPlayingSound() const;
};

static_assert(sizeof(SeadAudioActorWrapper) == 0xf0);
}  // namespace al
