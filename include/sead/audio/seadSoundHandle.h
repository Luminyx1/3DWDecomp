#pragma once

#include <nn/atk/atk_SoundHandle.h>

#include "basis/seadTypes.h"

namespace sead {
class SoundHandle : public nn::atk::SoundHandle {
public:
    void stop(s32 fadeFrames);
    void pause(s32 fadeFrames);
    void unpause(s32 fadeFrames);
    void setVolume(f32 volume, s32 frames);
    void setPitch(f32 pitch);
    void setPan(f32 pan);
    bool isAttachedSound() const;
    u32 getSoundId() const;
};
}  // namespace sead
