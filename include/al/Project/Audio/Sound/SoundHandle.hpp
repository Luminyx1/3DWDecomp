#pragma once

#include <audio/seadSoundHandle.h>

namespace al {
class IAcLSoundHandle {
public:
    virtual void detachSound() = 0;
    virtual void stop(s32 fadeFrames) = 0;
    virtual bool isAttachedSound() const = 0;
    virtual ~IAcLSoundHandle() {}
};

class IAcLSoundHandleObserver {
public:
    virtual ~IAcLSoundHandleObserver() {}
    virtual bool isAttachedSound() const = 0;
    virtual void detachSound() = 0;
    virtual void stop(s32 fadeFrames) = 0;
};

class AcLSoundHandle : public IAcLSoundHandle, public sead::SoundHandle, public IAcLSoundHandleObserver {
public:
    AcLSoundHandle();
    ~AcLSoundHandle() override;

    void detachSound() override;
    void stop(s32 fadeFrames) override;
    void pause(s32 fadeFrames);
    void unpause(s32 fadeFrames);
    void setVolume(f32 volume, s32 frames);
    bool isAttachedSound() const override;
};
}  // namespace al
