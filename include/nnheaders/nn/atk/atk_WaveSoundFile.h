#pragma once
#include <nn/atk/atk_BinaryFileFormat.h>

namespace nn::atk::detail {
struct AdshrCurve { u8 attack, decay, sustain, hold, release; };
struct WaveSoundFile {
    struct ReferenceTable { u32 count; Reference items[1]; };
    struct WaveId { u32 archiveId, waveIndex; };
    struct WaveIdTable { u32 count; WaveId waves[1]; };
    struct WaveSoundInfo {
        u32 flags, values[1];
        float GetPitch() const;
        u8 GetPan() const;
        u8 GetSurroundPan() const;
        void GetSendValue(u8* mainSend, u8* auxSends, u8 auxCount) const;
        const AdshrCurve* GetAdshrCurve() const;
        u8 GetLpfFreq() const;
        u8 GetBiquadType() const;
        u8 GetBiquadValue() const;
    };
    struct NoteInfo {
        u32 waveIndex, flags, values[1];
        float GetPitch() const;
        const AdshrCurve* GetAdshrCurve() const;
        u8 GetOriginalKey() const;
        u8 GetPan() const;
        u8 GetSurroundPan() const;
        u8 GetVolume() const;
        void GetSendValue(u8* mainSend, u8** auxSends, u8 auxCount) const;
    };
    struct NoteEvent;
    struct TrackInfo {
        Reference events;
        const ReferenceTable* GetNoteEventReferenceTable() const;
        const NoteEvent* GetNoteEvent(u32 index) const;
    };
    struct WaveSoundData {
        Reference info, tracks, notes;
        const WaveSoundInfo* GetWaveSoundInfo() const;
        const ReferenceTable* GetTrackInfoReferenceTable() const;
        const ReferenceTable* GetNoteInfoReferenceTable() const;
        const TrackInfo* GetTrackInfo(u32 index) const;
        const NoteInfo* GetNoteInfo(u32 index) const;
    };
    struct InfoBlockBody {
        Reference waveIds, sounds;
        const WaveSoundData* GetWaveSoundData(u32 index) const;
        const ReferenceTable* GetWaveSoundDataReferenceTable() const;
        const WaveIdTable* GetWaveIdTable() const;
    };
    struct InfoBlock { u32 signature, size; InfoBlockBody body; };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        const InfoBlock* GetInfoBlock() const;
    };
};
}
