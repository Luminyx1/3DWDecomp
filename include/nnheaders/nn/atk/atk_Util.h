#pragma once
#include <nn/types.h>

namespace nn::atk {
class SoundArchive;
namespace detail {
class SoundArchiveLoader;
namespace Util {
u32 CalcRandom();
const void* GetWaveFileOfWaveSound(const void* pWaveSoundFile, u32 index, const SoundArchive& rArchive,
                                   const SoundArchiveLoader& rLoader);
} // namespace Util
} // namespace detail
} // namespace nn::atk
