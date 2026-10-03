#include "System/Data/StampType.hpp"
#include "Project/Base/StringUtil.hpp"

/**
 * @brief Resolves a database type name to its numeric identifier.
 * @param pName Null-terminated type name compared using al::isEqualString.
 * @return Matching type identifier, or zero when no type matches.
 */
int StampType::calcStampTypeID(const char* pName) {
    if (isCourse(pName)) {
        return 0;
    }
    if (isDefault(pName)) {
        return 1;
    }
    if (isMarioComplete(pName)) {
        return 2;
    }
    if (isLuigiComplete(pName)) {
        return 3;
    }
    if (isPeachComplete(pName)) {
        return 4;
    }
    if (isKinopioComplete(pName)) {
        return 5;
    }
    if (isRosettaComplete(pName)) {
        return 6;
    }
    return 0;
}

/**
 * @brief Checks whether a database type name is Course.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isCourse(const char* pName) { return al::isEqualString(pName, "Course"); }

/**
 * @brief Checks whether a database type name is Default.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isDefault(const char* pName) { return al::isEqualString(pName, "Default"); }

/**
 * @brief Checks whether a database type name is MarioComplete.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isMarioComplete(const char* pName) { return al::isEqualString(pName, "MarioComplete"); }

/**
 * @brief Checks whether a database type name is LuigiComplete.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isLuigiComplete(const char* pName) { return al::isEqualString(pName, "LuigiComplete"); }

/**
 * @brief Checks whether a database type name is PeachComplete.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isPeachComplete(const char* pName) { return al::isEqualString(pName, "PeachComplete"); }

/**
 * @brief Checks whether a database type name is KinopioComplete.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isKinopioComplete(const char* pName) { return al::isEqualString(pName, "KinopioComplete"); }

/**
 * @brief Checks whether a database type name is RosettaComplete.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StampType::isRosettaComplete(const char* pName) { return al::isEqualString(pName, "RosettaComplete"); }
