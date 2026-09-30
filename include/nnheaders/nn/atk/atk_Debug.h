#pragma once
#include <nn/types.h>

namespace nn::atk {
enum DebugWarningFlag : int;
void Debug_SetWarningFlag(DebugWarningFlag flag, bool enabled);
namespace detail {
enum DebugSoundType { DebugSoundType_Sequence, DebugSoundType_Stream, DebugSoundType_Wave };
bool Debug_GetWarningFlag(DebugWarningFlag flag);
DebugWarningFlag Debug_GetDebugWarningFlagFromSoundType(DebugSoundType type);
const char* Debug_GetSoundTypeString(DebugSoundType type);
}
}
