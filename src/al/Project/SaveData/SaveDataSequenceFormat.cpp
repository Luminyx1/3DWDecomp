#include "Project/SaveData/SaveDataSequenceFormat.hpp"
#include "Project/SaveData/SaveDataSequenceInitDir.hpp"

namespace al {
/**
 * Sets the format parameters.
 * @param a First parameter.
 * @param b Second parameter.
 */
void SaveDataSequenceFormat::start(s32 a, s32 b) {
    _8 = a;
    _c = b;
}

/**
 * Formatting is unsupported.
 * @param pFileName Save file name.
 * @return Always -1.
 */
s32 SaveDataSequenceFormat::threadFunc(const char* pFileName) {
    return -1;
}

/**
 * Constructs the init dir sequence.
 * @param unk Unknown flag.
 */
SaveDataSequenceInitDir::SaveDataSequenceInitDir(u8 unk) : _18(unk) {}
}  // namespace al
