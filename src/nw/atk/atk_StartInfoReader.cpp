#include <nn/atk/atk_StartInfoReader.h>

namespace nn::atk::detail {
namespace {
using StartInfo = SoundStartable::StartInfo;

/**
 * @brief Test whether a playback setting was explicitly supplied.
 * @param rStartInfo Playback overrides whose enable mask is examined.
 * @param flag Setting's enable bit.
 * @return Whether the requested bit is set.
 */
inline bool HasStartFlag(const StartInfo& rStartInfo, StartInfo::EnableFlagBit flag) {
    return (rStartInfo.enableFlag & flag) != 0;
}
} // namespace

/**
 * @brief Initialize playback settings from the sound archive's defaults.
 * @param rSoundInfo Archive entry supplying the player, priority, and actor player slot.
 */
StartInfoReader::StartInfoReader(const SoundArchive::SoundInfo& rSoundInfo)
    : startOffsetType(StartInfo::StartOffsetType_MilliSeconds), startOffset(0), delayTime(0), delayCount(0),
      updateType(UpdateType_AudioFrame), playerPriority(rSoundInfo.playerPriority),
      playerId(rSoundInfo.playerId), actorPlayerId(rSoundInfo.actorPlayerId), pSequenceSoundInfo(nullptr),
      pStreamSoundInfo(nullptr), pStreamSoundMetaInfo(nullptr), pStreamSoundMetaInfo2(nullptr),
      pWaveSoundInfo(nullptr), pLoopInfo(nullptr), subMixIndex(0), pOutputReceiver(nullptr),
      isAdditionalDecodingOnLoopEnabled(true) {}

/**
 * @brief Apply enabled playback overrides, giving delay time precedence over delay count.
 * @param pStartInfo Optional overrides; null preserves all current settings. Referenced
 *                  sound and loop information must remain valid while this reader is used.
 */
void StartInfoReader::Read(const StartInfo* pStartInfo) {
    if (pStartInfo == nullptr) {
        return;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_StartOffset)) {
        startOffsetType = pStartInfo->startOffsetType;
        startOffset = pStartInfo->startOffset;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_PlayerPriority)) {
        playerPriority = pStartInfo->playerPriority;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_PlayerId)) {
        playerId = pStartInfo->playerId;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_ActorPlayerId)) {
        actorPlayerId = pStartInfo->actorPlayerId;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_SequenceSoundInfo)) {
        pSequenceSoundInfo = &pStartInfo->sequenceSoundInfo;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_StreamSoundInfo)) {
        pStreamSoundInfo = &pStartInfo->streamSoundInfo;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_StreamSoundMetaInfo)) {
        pStreamSoundMetaInfo = &pStartInfo->streamSoundMetaInfo;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_StreamSoundMetaInfo2)) {
        pStreamSoundMetaInfo2 = &pStartInfo->streamSoundMetaInfo2;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_WaveSoundInfo)) {
        pWaveSoundInfo = &pStartInfo->waveSoundInfo;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_DelayTime)) {
        delayTime = pStartInfo->delayTime;
    }
    if (!HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_DelayTime) &&
        HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_DelayCount)) {
        delayCount = pStartInfo->delayCount;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_UpdateType)) {
        updateType = pStartInfo->updateType;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_SubMixIndex)) {
        subMixIndex = pStartInfo->subMixIndex;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_OutputReceiver)) {
        pOutputReceiver = pStartInfo->pOutputReceiver;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_LoopInfo)) {
        pLoopInfo = &pStartInfo->loopInfo;
    }
    if (HasStartFlag(*pStartInfo, StartInfo::EnableFlagBit_IsAdditionalDecodingOnLoopEnabled)) {
        isAdditionalDecodingOnLoopEnabled = pStartInfo->isAdditionalDecodingOnLoopEnabled;
    }
}
} // namespace nn::atk::detail
