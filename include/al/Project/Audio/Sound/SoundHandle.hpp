#pragma once

#include <audio/seadSoundHandle.h>
#include <basis/seadTypes.h>

namespace al {
class IAcLSoundHandle {
public:
    virtual ~IAcLSoundHandle() = default;
    virtual bool isAttachedSound() const = 0;
    virtual void detachSound() = 0;
    virtual void stop(s32 fadeFrames) = 0;
};

class AcLSoundHandlePlatform : public sead::SoundHandle {
public:
    AcLSoundHandlePlatform();

    virtual void detachSound();

    bool isPause() const;
    f32 getVolume() const;
    void setSurroundPan(f32 pan);
    void setMainSend(f32 send);
    void setFxSend(s32 bus, f32 send);
    void setLpfFreq(f32 freq);
    void setBiquadFilter(s32 type, f32 value);
    static bool readSeqGlobalVariable(s32 index, s16* pVar);
    static bool writeSeqGlobalVariable(s32 index, s16 value);
    bool readSeqLocalVariable(s32 index, s16* pVar);
    bool writeSeqLocalVariable(s32 index, s16 value);
    bool readSeqTrackVariable(s32 track, s32 index, s16* pVar);
    bool writeSeqTrackVariable(s32 track, s32 index, s16 value);
    void setSeqTempoRatio(f32 ratio);
    void setOutputDeviceSpeakerVolume(s32 device, f32 frontLeft, f32 frontRight, f32 rearLeft,
                                      f32 rearRight, f32 frontCenter, f32 lfe);
    void setAllOutputDeviceSpeakerVolume(f32 frontLeft, f32 frontRight, f32 rearLeft, f32 rearRight,
                                         f32 frontCenter, f32 lfe);

    sead::SoundHandle* getSoundHandle() { return this; }
};

class AcLSoundHandle : public AcLSoundHandlePlatform, public IAcLSoundHandle {
public:
    AcLSoundHandle();

    void detachSound() override;
    void stop(s32 fadeFrames) override;
    bool isAttachedSound() const override { return sead::SoundHandle::isAttachedSound(); }
    ~AcLSoundHandle() override = default;

    void pause(s32 fadeFrames);
    void unpause(s32 fadeFrames);
    void setVolume(f32 volume, s32 fadeFrames);
};
}  // namespace al
