#pragma once

#include "basis/seadTypes.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead {
class AudioMgr;

class AudioResourceLoader {
    SEAD_RTTI_BASE(AudioResourceLoader)

public:
    AudioResourceLoader() = default;
    virtual ~AudioResourceLoader() {}

    virtual void initialize(AudioMgr& rMgr) = 0;
    virtual void load() = 0;
    virtual void finalize() = 0;
};
}  // namespace sead
