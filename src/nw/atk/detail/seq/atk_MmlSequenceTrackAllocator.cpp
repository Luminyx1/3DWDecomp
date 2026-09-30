#include <nn/atk/atk_MmlSequenceTrack.h>
#include <new>

namespace nn::atk::detail::driver {
// player receives ownership of a track initialized with this allocator's parser.
SequenceTrack* MmlSequenceTrackAllocator::AllocTrack(SequenceSoundPlayer* player) {
    void* memory = mPool.AllocImpl();
    auto* track = memory ? new (memory) MmlSequenceTrack : nullptr;

    if (track) {
        track->SetPlayer(player);
        track->SetParser(mParser);
    }

    return track;
}

// track is detached and destroyed before its storage is returned to the pool.
void MmlSequenceTrackAllocator::FreeTrack(SequenceTrack* track) {
    track->SetPlayer(nullptr);

    if (track) {
        track->~SequenceTrack();
        mPool.FreeImpl(track);
    }
}

// memory provides size bytes of caller-owned storage for fixed-size MML tracks.
int MmlSequenceTrackAllocator::Create(void* memory, size_t size) {
    return mPool.CreateImpl(memory, size, sizeof(MmlSequenceTrack));
}

void MmlSequenceTrackAllocator::Destroy() { mPool.DestroyImpl(); }
int MmlSequenceTrackAllocator::GetAllocatableTrackCount() const { return mPool.CountImpl(); }
}
