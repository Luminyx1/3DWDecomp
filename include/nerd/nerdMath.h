#pragma once

#include <basis/seadTypes.h>

/// Fast math routines the NX build of sead's f32 math goes through.
namespace nerd {
    void setUseFastsqrte(bool);
    f32 sqrt(f32);
    f32 rsqrt(f32);
}  // namespace nerd
