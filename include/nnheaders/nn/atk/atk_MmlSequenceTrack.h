#pragma once
#include <nn/atk/atk_InstancePool.h>

namespace nn::atk::detail::driver {
class SequenceSoundPlayer;
class MmlSequenceTrack;
class MmlParser {
public:
    int Parse(MmlSequenceTrack* track, bool playNotes) const;
};
class SequenceTrack {
public:
    SequenceTrack();
    virtual ~SequenceTrack();
    virtual int Parse(bool playNotes) = 0;
    // player owns the track; null detaches it before returning its storage to the pool.
    void SetPlayer(SequenceSoundPlayer* player) { mPlayer = player; }
private:
    u8 _8[0x1d8 - 8];
    SequenceSoundPlayer* mPlayer;
    u8 _1e0[8];
};
class MmlSequenceTrack : public SequenceTrack {
public:
    MmlSequenceTrack();
    int Parse(bool playNotes) override;
    // parser interprets this track's sequence commands.
    void SetParser(const MmlParser* parser) { mParser = parser; }
private:
    const MmlParser* mParser;
};
class SequenceTrackAllocator {
public:
    virtual ~SequenceTrackAllocator() {}
    virtual SequenceTrack* AllocTrack(SequenceSoundPlayer* player) = 0;
    virtual void FreeTrack(SequenceTrack* track) = 0;
    virtual int GetAllocatableTrackCount() const = 0;
};
class MmlSequenceTrackAllocator : public SequenceTrackAllocator {
public:
    SequenceTrack* AllocTrack(SequenceSoundPlayer* player) override;
    void FreeTrack(SequenceTrack* track) override;
    int GetAllocatableTrackCount() const override;
    int Create(void* memory, size_t size);
    void Destroy();
private:
    const MmlParser* mParser;
    PoolImpl mPool;
};
static_assert(sizeof(SequenceTrack) == 0x1e8, "Sequence track size");
static_assert(sizeof(MmlSequenceTrack) == 0x1f0, "MML sequence track size");
static_assert(sizeof(MmlSequenceTrackAllocator) == 0x28, "MML track allocator size");
}
