#pragma once

#include "audio/seadAudioSubsetBase.h"

namespace sead {
class MicMgrCafe : public AudioSubsetBase {
    SEAD_RTTI_OVERRIDE(MicMgrCafe, AudioSubsetBase)

public:
    MicMgrCafe() = default;
    ~MicMgrCafe() override {}

    void initialize(AudioMgr& rMgr, Heap* pHeap) override {}
    void finalize() override {}
    void calc() override {}
};
}  // namespace sead
