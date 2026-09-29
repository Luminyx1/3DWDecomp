#pragma once

#include <nn/atk/atk_Sound3DActor.h>

#include "audio/seadAudioGlobal.h"
#include "basis/seadTypes.h"
#include "math/seadVector.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class Audio3DMgr;
class SoundHandle;

class Audio3DActor {
    SEAD_RTTI_BASE(Audio3DActor)

public:
    Audio3DActor() = default;
    virtual ~Audio3DActor();

    virtual void initialize(Audio3DMgr* pMgr) = 0;
    virtual void finalize() = 0;
    virtual void setPosition(const Vector3f& rPosition) = 0;
    virtual void resetPosition() = 0;
    virtual bool startSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) = 0;
    virtual bool startSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) = 0;
    virtual bool holdSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) = 0;
    virtual bool holdSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) = 0;
};

class Audio3DActorNin : public Audio3DActor, public nn::atk::Sound3DActor {
    SEAD_RTTI_OVERRIDE(Audio3DActorNin, Audio3DActor)

public:
    Audio3DActorNin();
    ~Audio3DActorNin() override {}

    void initialize(Audio3DMgr* pMgr) override;
    void finalize() override;
    void setPosition(const Vector3f& rPosition) override;
    void resetPosition() override;
    bool startSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) override;
    bool startSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) override;
    bool holdSound(SoundHandle* pHandle, u32 soundId, AudioStartResult* pResult) override;
    bool holdSound(SoundHandle* pHandle, const char* pSoundName, AudioStartResult* pResult) override;
    virtual void setVelocity(const Vector3f& rVelocity);
    virtual Vector3f getVelocity() const;
    virtual void stopAllSound(s32 fadeFrames);
    virtual void pauseAllSound(bool pause, s32 fadeFrames);

    StartResult SetupSound(nn::atk::SoundHandle* pHandle, u32 soundId, const StartInfo* pStartInfo,
                           void* pSetupArg) override;

private:
    bool mIsStartDisabled = false;
};
static_assert(sizeof(Audio3DActorNin) == 0x130);
}  // namespace sead
