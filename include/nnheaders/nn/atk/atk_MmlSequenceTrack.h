#pragma once
#include <nn/atk/atk_InstancePool.h>

namespace nn::atk { enum SequenceMute : int; }
namespace nn::atk::detail::driver {
class SequenceTrack;
class Channel;
template <typename T> struct MmlMoveValue {
    T origin, target;
    s16 frame, counter;
    T GetValue() const {
        return counter >= frame ? target : static_cast<T>(origin + (target - origin) * counter / frame);
    }
    // value is the new target and frames is the interpolation duration.
    void Set(T value, int frames) {
        origin = GetValue(); target = value; frame = frames; counter = 0;
    }
};
class SequenceSoundPlayer {
public:
    s16* GetVariablePtr(int index);
    SequenceTrack* GetPlayerTrack(int index);
    void CallSequenceUserprocCallback(u16 id, SequenceTrack* track);
private:
    friend class MmlParser;
    u8 _0[0x111];
    u8 mParamB0;
    u16 mTempo;
    MmlMoveValue<u8> mVolume;
};
class MmlSequenceTrack;
class MmlParser {
public:
    enum SeqArgType { SeqArgType_None, SeqArgType_Byte, SeqArgType_Short, SeqArgType_VariableLength,
                      SeqArgType_Random, SeqArgType_Variable };
    static bool mPrintVarEnabledFlag;
    virtual ~MmlParser();
    virtual void CommandProc(MmlSequenceTrack* track, u32 command, int first, int second) const;
    virtual Channel* NoteOnCommandProc(MmlSequenceTrack* track, int key, int velocity, int length, bool tie) const;
    int Parse(MmlSequenceTrack* track, bool playNotes) const;
    int ReadArg(const u8** position, SequenceSoundPlayer* player, SequenceTrack* track, SeqArgType type) const;
    u32 Read24(const u8** position) const;
    u16 Read16(const u8** position) const;
    u32 ReadVar(const u8** position) const;
    s16* GetVariablePtr(SequenceSoundPlayer* player, SequenceTrack* track, int index) const;
    static u32 ParseAllocTrack(const void* data, u32 offset, u32* trackMask);
};
class SequenceTrack {
public:
    SequenceTrack();
    virtual ~SequenceTrack();
    virtual int Parse(bool playNotes) = 0;
    s16* GetVariablePtr(int index);
    Channel* NoteOn(int key, int velocity, int length, bool tie);
    void Close();
    void Open();
    void SetSeqData(const void* data, int offset);
    void ReleaseAllChannel(int release);
    void FreeAllChannel();
    void SetMute(SequenceMute mute);
    // player owns the track; null detaches it before returning its storage to the pool.
    void SetPlayer(SequenceSoundPlayer* player) { mPlayer = player; }
private:
    friend class MmlParser;
    u8 _8[0x68 - 8];
    struct SequenceContext {
        const u8* mSequenceData;
        const u8* mPosition;
        bool mCondition;
        bool mNoteWait;
        bool mTie;
        bool mMono;
        u8 _7c[4];
        struct StackEntry { bool isLoop; u8 count; const u8* position; };
        StackEntry mStack[10];
        u8 mStackDepth;
        bool mParamBF;
        bool mMuted;
        u8 _123;
        int mWait;
        // Push the current cursor as a return address for a sequence subroutine.
        void PushCall() {
            const u8* position = mPosition;
            auto& entry = mStack[mStackDepth];
            entry.isLoop = false;
            entry.position = position;
            ++mStackDepth;
        }
    } mContext;
    bool mWaitForNote;
    bool mPortamento;
    bool mParamDF;
    u8 mParamB6;
    u32 mProgram;
    float mPitchSweep;
    MmlMoveValue<u8> mVolume;
    MmlMoveValue<u8> mVolume2;
    MmlMoveValue<s8> mPan;
    MmlMoveValue<s8> mSurroundPan;
    MmlMoveValue<s8> mPitchBend;
    u8 _152[2];
    struct LfoParam {
        float depth, speed;
        int delay;
        u8 type, target, range, reserved;

    };
    LfoParam mLfo[4];
    u8 mLfoShape[4];
    u8 mParamB3;
    u8 mBendRange;
    s8 mParamDC;
    u8 _19b;
    s8 mTranspose;
    u8 mPriority;
    u8 mPortamentoKey;
    u8 mPortamentoTime;
    s8 mAttack, mDecay, mSustain, mRelease;
    u16 mHold;
    u8 mParamB4;
    u8 mParamDB;
    u8 mParamD9;
    u8 mParamDA;
    u8 mParamDE;
    u8 _1ab;
    float mParamD8;
    float mParamB5;
    u8 _1b4[0x1d8 - 0x1b4];
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
