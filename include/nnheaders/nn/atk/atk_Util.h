#pragma once
#include <nn/types.h>

namespace nn::atk {
class SoundArchive;
namespace detail {
class SoundArchiveLoader;
struct LoadItemInfo;
namespace Util {
/** @brief Outcome of resolving the wave archive a bank plays from. */
enum WaveArchiveLoadStatus {
    WaveArchiveLoadStatus_Error = -1,
    WaveArchiveLoadStatus_Ok,
    WaveArchiveLoadStatus_Noneed,
    WaveArchiveLoadStatus_NotYet,
};
u32 CalcRandom();
const void* GetWaveFileOfWaveSound(const void* pWaveSoundFile, u32 index, const SoundArchive& rArchive,
                                   const SoundArchiveLoader& rLoader);
WaveArchiveLoadStatus GetWaveArchiveOfBank(LoadItemInfo& rWarcInfo, bool& rIsLoadIndividual,
                                           const void* pBankFile, const SoundArchive& rArchive,
                                           const SoundArchiveLoader& rLoader);
} // namespace Util
} // namespace detail
} // namespace nn::atk
