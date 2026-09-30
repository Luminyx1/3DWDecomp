#include <nn/atk/atk_Debug.h>

namespace nn::atk {
namespace {
u32 warningFlags = 31;
const u32 warningMasks[] = {31, 1, 2, 4, 8, 16};
const char* soundTypeNames[] = {"seq", "strm", "wave"};
// flag zero selects all five warning bits; flags one through five select one bit.
u32 GetWarningMask(DebugWarningFlag flag) {
    return static_cast<u32>(flag) <= 5 ? warningMasks[static_cast<int>(flag)] : 0;
}
}

// flag selects a warning category (zero selects all); enabled sets or clears its bits.
void Debug_SetWarningFlag(DebugWarningFlag flag, bool enabled) {
    u32 mask = GetWarningMask(flag);
    if (enabled) warningFlags |= mask;
    else warningFlags &= ~mask;
}

namespace detail {
// flag selects the bits to inspect. An unknown flag has an empty mask and returns true.
bool Debug_GetWarningFlag(DebugWarningFlag flag) {
    u32 mask = GetWarningMask(flag);
    return (warningFlags & mask) == mask;
}

// type selects sequence, stream, or wave warnings; unknown types use sequence warnings.
DebugWarningFlag Debug_GetDebugWarningFlagFromSoundType(DebugSoundType type) {
    switch (type) {
    case DebugSoundType_Wave: return static_cast<DebugWarningFlag>(3);
    case DebugSoundType_Stream: return static_cast<DebugWarningFlag>(2);
    default: return static_cast<DebugWarningFlag>(1);
    }
}

// type selects a short diagnostic name; unknown types return an empty string.
const char* Debug_GetSoundTypeString(DebugSoundType type) {
    return static_cast<u32>(type) <= 2 ? soundTypeNames[static_cast<int>(type)] : "";
}
}
}
