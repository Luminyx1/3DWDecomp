#include "audio/seadAudioSoundHeapNin.h"

#include "heap/seadHeapMgr.h"
#include "hostio/seadHostIOPropertyEvent.h"

namespace sead {
/**
 * Constructs a sound heap and creates it.
 * @param size Size of the heap, or 0 to use all of the allocatable memory.
 * @param pHeap Heap to allocate from, or nullptr for the current heap.
 */
AudioSoundHeapNin::AudioSoundHeapNin(size_t size, Heap* pHeap) {
    create_(size, pHeap);
}

/**
 * Allocates the heap buffer and creates the sound heap on it.
 * @param size Size of the heap, or 0 to use all of the allocatable memory.
 * @param pHeap Heap to allocate from, or nullptr for the current heap.
 */
void AudioSoundHeapNin::create_(size_t size, Heap* pHeap) {
    if (!pHeap) {
        pHeap = HeapMgr::instance()->getCurrentHeap();
    }
    if (size == 0) {
        size = pHeap->getMaxAllocatableSize(4) - 32;
    }
    mBuffer = new (pHeap, 4) u8[size];
    Create(mBuffer, size);
}

/**
 * Destroys the sound heap and frees its buffer.
 */
AudioSoundHeapNin::~AudioSoundHeapNin() {
    Destroy();
    if (mBuffer) {
        delete[] mBuffer;
        mBuffer = nullptr;
    }
}

/**
 * Sets the data used when dumping the heap.
 * @param rMgr Sound data manager.
 * @param rArchive Sound archive.
 */
void AudioSoundHeapNin::setSoundDataManagement(nn::atk::SoundDataManager& rMgr,
                                               nn::atk::SoundArchive& rArchive) {
    mSoundDataManager = &rMgr;
    mSoundArchive = &rArchive;
}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void AudioSoundHeapNin::genMessage(hostio::Context* pContext) {}

/**
 * Dumps the heap when the dump command is received.
 * @param pEvent Property event.
 */
void AudioSoundHeapNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {
    if (pEvent->getIdValue() == 0x64756d70) {
        dump();
    }
}

/**
 * Dumps the contents of the heap.
 */
void AudioSoundHeapNin::dump() {
    if (IsValid() && mSoundDataManager && mSoundArchive) {
        Dump(*mSoundDataManager, *mSoundArchive);
    }
}
}  // namespace sead
