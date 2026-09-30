/**
 * @file EmitterSet.h
 * @brief VFX emitter set.
 */

#pragma once

#include <nn/types.h>

namespace nn {
namespace vfx {
class EmitterSet {
public:
    s32 GetCreateId() const { return m_CreateId; }
    void SetDirectionalVel(f32 vel) { m_DirectionalVel = vel; }

    u8 _0[0x2c];
    s32 m_CreateId;
    u8 _30[0x210 - 0x30];
    f32 m_DirectionalVel;
};
}  // namespace vfx
}  // namespace nn
