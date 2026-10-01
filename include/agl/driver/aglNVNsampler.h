#pragma once

#include <nvn/nvn.h>
#include <basis/seadTypes.h>

namespace agl::driver {

class NVNsampler_ {
public:
    NVNsampler_();
    NVNsampler_(const NVNsampler_& other);
    ~NVNsampler_();

    NVNsampler_& operator=(const NVNsampler_& other);

    void releaseSampler();
    bool registerSampler(const NVNsampler& sampler, const char*);
    void updateTextureId(s32);

    u64 getHandle() const { return reinterpret_cast<u64>(_0); }
    s32 getSamplerID() const { return _8; }

private:
    void* _0;
    s16 _8;
};

}  // namespace agl::driver
