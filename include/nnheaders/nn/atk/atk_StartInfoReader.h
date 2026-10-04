#pragma once
#include <nn/atk/atk_SoundStartable.h>

namespace nn::atk::detail {
class StartInfoReader {
  public:
    using StartInfo = SoundStartable::StartInfo;

    explicit StartInfoReader(const SoundArchive::SoundInfo& rSoundInfo);
    void Read(const StartInfo* pStartInfo);

    StartInfo::StartOffsetType startOffsetType;
    int startOffset;
    int delayTime;
    int delayCount;
    UpdateType updateType;
    int playerPriority;
    SoundArchive::ItemId playerId;
    int actorPlayerId;
    const StartInfo::SequenceSoundInfo* pSequenceSoundInfo;
    const StartInfo::StreamSoundInfo* pStreamSoundInfo;
    const SoundArchive::StreamSoundInfo* pStreamSoundMetaInfo;
    const SoundArchive::StreamSoundInfo2* pStreamSoundMetaInfo2;
    const StartInfo::WaveSoundInfo* pWaveSoundInfo;
    const StartInfo::LoopInfo* pLoopInfo;
    int subMixIndex;
    OutputReceiver* pOutputReceiver;
    bool isAdditionalDecodingOnLoopEnabled;
};
static_assert(sizeof(StartInfoReader) == 0x68, "StartInfoReader size");
} // namespace nn::atk::detail
