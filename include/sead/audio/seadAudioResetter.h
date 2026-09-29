#pragma once

#include "basis/seadTypes.h"

namespace sead {
class AudioMgr;

class AudioResetter {
public:
    AudioResetter();
    virtual ~AudioResetter() {}

    virtual void initialize(AudioMgr& rMgr);
    virtual void calc() = 0;
    virtual void reset(s32 fadeFrames);
    virtual bool isResetting() const;
    virtual bool isResetDone() const;
    virtual void recoverReset();
    virtual void shutdown(s32 fadeFrames);
    virtual bool isShuttingDown() const;
    virtual bool isShutdownDone() const;

protected:
    AudioMgr* mAudioMgr = nullptr;
};

class AudioResetterNin : public AudioResetter {
public:
    AudioResetterNin();
    ~AudioResetterNin() override {}

    void initialize(AudioMgr& rMgr) override;
    void calc() override;
    void reset(s32 fadeFrames) override;
    bool isResetting() const override;
    bool isResetDone() const override;
    void recoverReset() override;
    void shutdown(s32 fadeFrames) override;
    bool isShuttingDown() const override;
    bool isShutdownDone() const override;

private:
    enum State {
        cState_None = 0,
        cState_Running = 1,
        cState_Done = 2
    };

    s32 mResetState = cState_None;
    s32 mShutdownState = cState_None;
    f32 mMasterVolume = 1.0f;
};
static_assert(sizeof(AudioResetterNin) == 0x20);
}  // namespace sead
