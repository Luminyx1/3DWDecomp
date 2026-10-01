/**
 * @file Handle.h
 * @brief VFX emitter set handle.
 */

#pragma once

#include <nn/types.h>
#include <nn/vfx/EmitterSet.h>

namespace nn {
namespace vfx {
class Handle {
public:
    EmitterSet* GetEmitterSet();

    bool IsValid() const {
        return m_EmitterSet != nullptr && m_CreateId == m_EmitterSet->GetCreateId();
    }

    void Invalidate() {
        m_EmitterSet = nullptr;
        m_CreateId = -1;
    }

    EmitterSet* m_EmitterSet;
    s32 m_CreateId;
};
}  // namespace vfx
}  // namespace nn
