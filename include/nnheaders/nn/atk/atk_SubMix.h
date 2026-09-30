#pragma once
#include <nn/atk/atk_FinalMix.h>

namespace nn::atk {
class SubMix : public OutputMixer {
public:
    audio::SubMixType* GetSubMix() { return &mSubMix; }
private:
    util::IntrusiveListNode mLinkNode;
    audio::SubMixType mSubMix;
    // Remaining routing, volume, synchronization, and state fields await reconstruction.
    u8 _58[0xf8 - 0x58];
};
}
