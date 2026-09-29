#pragma once

#include <nn/atk/atk_SoundHeap.h>

#include "basis/seadTypes.h"
#include "hostio/seadHostIONode.h"

namespace sead {
class Heap;

namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio

class AudioSoundHeapNin : public nn::atk::SoundHeap, public hostio::Node {
public:
    AudioSoundHeapNin(size_t size, Heap* pHeap);
    ~AudioSoundHeapNin() override;

    void setSoundDataManagement(nn::atk::SoundDataManager& rMgr, nn::atk::SoundArchive& rArchive);
    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);
    void dump();

private:
    void create_(size_t size, Heap* pHeap);

    u8* mBuffer = nullptr;
    nn::atk::SoundDataManager* mSoundDataManager = nullptr;
    nn::atk::SoundArchive* mSoundArchive = nullptr;
};
static_assert(sizeof(AudioSoundHeapNin) == 0x70);
}  // namespace sead
