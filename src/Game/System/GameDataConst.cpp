#include "System/GameDataConst.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
const char* const cCharacterNames[] = {"Mario", "Luigi", "Peach", "Kinopio", "Rosetta"};
const int cPadPorts[] = {1, 2, 3, 4};
} // namespace

/**
 * @brief Gets the maximum ghost-save allocation size.
 * @return Maximum ghost-save size in bytes.
 */
unsigned int GameDataConst::getSaveDataSizeGhostMax() { return 0x32f3c8; }

/**
 * @brief Gets the ghost-save format version.
 * @return Ghost-save version zero.
 */
int GameDataConst::getSaveDataVersionGhost() { return 0; }

/**
 * @brief Gets the database name of a playable character.
 * @param characterType Character identifier from 0 to 4; not bounds-checked.
 * @return Static character name.
 */
const char* GameDataConst::getPlayerCharacterName(int characterType) {
    return cCharacterNames[characterType];
}

/**
 * @brief Resolves a playable character name.
 * @param pName Null-terminated character name to resolve.
 * @return Character identifier from 0 to 4, or -1 when not found.
 */
int GameDataConst::getPlayerCharacterTypeFromName(const char* pName) {
    if (al::isEqualString(cCharacterNames[0], pName)) {
        return 0;
    }
    if (al::isEqualString(cCharacterNames[1], pName)) {
        return 1;
    }
    if (al::isEqualString(cCharacterNames[2], pName)) {
        return 2;
    }
    if (al::isEqualString(cCharacterNames[3], pName)) {
        return 3;
    }
    if (al::isEqualString(cCharacterNames[4], pName)) {
        return 4;
    }
    return -1;
}

/**
 * @brief Gets the number of supported controller ports.
 * @return Four controller ports.
 */
int GameDataConst::getPadPortListNum() { return 4; }

/**
 * @brief Gets the default controller-port ordering.
 * @return Static array of four controller port identifiers.
 */
const int* GameDataConst::getPadPortList() { return cPadPorts; }
