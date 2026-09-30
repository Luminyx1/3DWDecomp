#pragma once

#include <nn/types.h>

namespace nn::g3d {

// TODO
class ResRenderState {
public:
    s32 GetMode() const { return m_Mode; }

private:
    u32 m_Flag;
    s32 m_Mode;
};

}  // namespace nn::g3d
