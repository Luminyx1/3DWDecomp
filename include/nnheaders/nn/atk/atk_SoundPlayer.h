#pragma once

#include <nn/types.h>

namespace nn::atk {
namespace detail { class PlayerHeap; }
class SoundPlayer {
public:
    detail::PlayerHeap* detail_AllocPlayerHeap();
    void detail_FreePlayerHeap(detail::PlayerHeap* pHeap);
    void StopAllSound(int fadeFrames);
    void PauseAllSound(bool flag, int fadeFrames);
};
}  // namespace nn::atk
