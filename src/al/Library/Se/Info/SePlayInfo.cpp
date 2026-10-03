#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Math/InOutParam.hpp"
#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {

/**
 * @brief Compares two SE play information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SePlayInfo::compareInfo(const SePlayInfo* pA, const SePlayInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

} // namespace al
