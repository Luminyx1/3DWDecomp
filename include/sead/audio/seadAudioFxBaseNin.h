#pragma once

#include <nn/atk/atk_EffectAux.h>
#include <nn/audio.h>

#include "basis/seadTypes.h"
#include "hostio/seadHostIONode.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class AudioFxParam : public hostio::Node {
    SEAD_RTTI_BASE(AudioFxParam)

public:
    AudioFxParam();
    virtual ~AudioFxParam() {}
};

class AudioFxBaseNin : public nn::atk::EffectAux {
    SEAD_RTTI_BASE(AudioFxBaseNin)

public:
    AudioFxBaseNin();
    ~AudioFxBaseNin() override;

    bool Initialize() override;
    void Finalize() override;

    virtual size_t GetRequiredMemSize() const;
    virtual bool AssignWorkBuffer(void* pBuffer, u32 size);
    virtual void ReleaseWorkBuffer();

    void* getWorkBuffer() const { return mWorkBuffer; }
    u32 getWorkBufferSize() const { return mWorkBufferSize; }

protected:
    bool mIsMemoryPoolAttached;
    nn::audio::MemoryPoolType mMemoryPool;
    void* mWorkBuffer;
    u32 mWorkBufferSize;
    void* mFxWorkBuffer;
};
static_assert(sizeof(AudioFxBaseNin) == 0x90);
}  // namespace sead
