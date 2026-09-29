#pragma once

#include "basis/seadTypes.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class AudioFxBaseNin;

class IAudioFxImpl {
public:
    virtual ~IAudioFxImpl() {}
    virtual void _10() = 0;
    virtual void _18() = 0;
    virtual void _20() = 0;
    virtual void _28() = 0;
    virtual AudioFxBaseNin* getFxNin() = 0;
};

class AudioFx {
    SEAD_RTTI_BASE(AudioFx)

public:
    virtual ~AudioFx() {}
};

class AudioFxNin : public AudioFx {
    SEAD_RTTI_OVERRIDE(AudioFxNin, AudioFx)

public:
    IAudioFxImpl* getImpl() const { return mImpl; }

private:
    void* _8;
    IAudioFxImpl* mImpl;
};

class AudioFxObject {
public:
    virtual ~AudioFxObject() {}

    IAudioFxImpl* getImpl() const { return mImpl; }

private:
    IAudioFxImpl* mImpl;
};
}  // namespace sead
