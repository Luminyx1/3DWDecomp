#pragma once

#include "audio/seadAudioSubsetBase.h"

namespace sead {
class Audio3DMgr : public AudioSubsetBase {
    SEAD_RTTI_OVERRIDE(Audio3DMgr, AudioSubsetBase)

public:
    Audio3DMgr() = default;
    ~Audio3DMgr() override;

    void initialize(AudioMgr& rMgr, Heap* pHeap) override {}
    void finalize() override {}
};
}  // namespace sead
