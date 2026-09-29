#pragma once

#include "audio/seadAudioGlobal.h"
#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class AudioFx;
class AudioFxObject;

class ISoundFrameCallback {
public:
    virtual void onSoundFrame() = 0;

private:
    friend class AudioSystemNin;

    ListNode mListNode;
};

class AudioSystem {
    SEAD_RTTI_BASE(AudioSystem)

public:
    AudioSystem() = default;
    virtual ~AudioSystem() {}

    virtual void initialize() = 0;
    virtual void finalize() = 0;
    virtual bool setOutputMode(AudioGlobal::OutputMode mode) = 0;
    virtual AudioGlobal::OutputMode getOutputMode() const = 0;
    virtual bool appendEffect(AudioGlobal::AuxBus bus, AudioFx* pFx) = 0;
    virtual bool appendFxObject(AudioGlobal::AuxBus bus, AudioFxObject* pFxObject) = 0;
    virtual void clearEffect(AudioGlobal::AuxBus bus, s32 fadeFrames) = 0;
    virtual bool isFinishedClearEffect(AudioGlobal::AuxBus bus) = 0;
    virtual void appendSoundFrameCallback(ISoundFrameCallback& rCallback) = 0;
    virtual void removeSoundFrameCallback(ISoundFrameCallback& rCallback) = 0;
    virtual void clearSoundFrameCallback() = 0;
};
}  // namespace sead
