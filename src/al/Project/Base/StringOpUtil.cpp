#include "Project/Base/StringOpUtil.hpp"

#include <cstring>

namespace al {
/**
 * Gets the part of a path after the last slash.
 * @param pName The path.
 * @return The base name.
 */
const char* getBaseName(const char* pName) {
    const char* baseName = strrchr(pName, '/');
    return baseName != nullptr ? baseName + 1 : pName;
}
}  // namespace al
