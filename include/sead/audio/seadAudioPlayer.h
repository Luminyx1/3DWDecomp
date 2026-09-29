#pragma once

#include "basis/seadTypes.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class SoundHandle;

class AudioPlayer {
    SEAD_RTTI_BASE(AudioPlayer)

public:
    AudioPlayer() = default;
    virtual ~AudioPlayer() {}

    virtual void initialize() {}
    virtual void finalize() {}
    virtual void calc() {}
    virtual bool startSound(SoundHandle* pHandle, u32 soundId) { return false; }
    virtual bool startSound(SoundHandle* pHandle, const char* pSoundName) { return false; }
    virtual bool holdSound(SoundHandle* pHandle, u32 soundId) { return false; }
    virtual bool holdSound(SoundHandle* pHandle, const char* pSoundName) { return false; }
    virtual u32 getSoundCount() const { return 0; }
    virtual const char* getSoundName(u32 soundId) const { return nullptr; }
    virtual u32 getSoundId(const char* pSoundName) const { return 0xffffffff; }
};
}  // namespace sead
