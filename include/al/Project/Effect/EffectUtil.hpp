#pragma once

#include <gfx/seadColor.h>

namespace nn::vfx::detail {
struct ResAnim8KeyParamSet;
}  // namespace nn::vfx::detail

namespace al {
void calcAnim8Key(sead::Color4f* pOut, const nn::vfx::detail::ResAnim8KeyParamSet& rParam,
                  f32 life, f32 frame);
}  // namespace al
