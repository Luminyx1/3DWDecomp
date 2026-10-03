#include "System/Data/StageType.hpp"
#include "Project/Base/StringUtil.hpp"

/**
 * @brief Resolves a database type name to its numeric identifier.
 * @param pName Null-terminated type name compared using al::isEqualString.
 * @return Matching type identifier, or zero when no type matches.
 */
int StageType::calcStageTypeID(const char* pName) {
    if (isNormal(pName)) {
        return 0;
    }
    if (isDrcOnly(pName)) {
        return 1;
    }
    if (isGoldenExpress(pName)) {
        return 2;
    }
    if (isKinopioBrigade(pName)) {
        return 3;
    }
    if (isContinuousMysteryBox(pName)) {
        return 4;
    }
    if (isKinopioHouse(pName)) {
        return 5;
    }
    if (isGateKeeperGoalPole(pName)) {
        return 6;
    }
    if (isGateKeeperNoGoalPole(pName)) {
        return 7;
    }
    if (isKoopaCastleNormal(pName)) {
        return 8;
    }
    if (isKoopaCastleTank(pName)) {
        return 9;
    }
    if (isKoopaCastleExpress(pName)) {
        return 10;
    }
    if (isKoopaCastleExpressNormal(pName)) {
        return 11;
    }
    if (isKoopaCastleFortress(pName)) {
        return 12;
    }
    if (isCasinoRoom(pName)) {
        return 13;
    }
    if (isFairyHouse(pName)) {
        return 14;
    }
    if (isKinopioHouseHide(pName)) {
        return 15;
    }
    if (isDokanHide(pName)) {
        return 16;
    }
    if (isChampionShip(pName)) {
        return 17;
    }
    return 0;
}

/**
 * @brief Checks whether a database type name is 通常.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isNormal(const char* pName) { return al::isEqualString(pName, "\u901a\u5e38"); }

/**
 * @brief Checks whether a database type name is DRC専用.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isDrcOnly(const char* pName) { return al::isEqualString(pName, "DRC\u5c02\u7528"); }

/**
 * @brief Checks whether a database type name is ゴールデンエクスプレス.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isGoldenExpress(const char* pName) {
    return al::isEqualString(pName, "\u30b4\u30fc\u30eb\u30c7\u30f3\u30a8\u30af\u30b9\u30d7\u30ec\u30b9");
}

/**
 * @brief Checks whether a database type name is キノピオ探検隊.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKinopioBrigade(const char* pName) {
    return al::isEqualString(pName, "\u30ad\u30ce\u30d4\u30aa\u63a2\u691c\u968a");
}

/**
 * @brief Checks whether a database type name is ミステリーハウス.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isContinuousMysteryBox(const char* pName) {
    return al::isEqualString(pName, "\u30df\u30b9\u30c6\u30ea\u30fc\u30cf\u30a6\u30b9");
}

/**
 * @brief Checks whether a database type name is キノピオの家.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKinopioHouse(const char* pName) {
    return al::isEqualString(pName, "\u30ad\u30ce\u30d4\u30aa\u306e\u5bb6");
}

/**
 * @brief Checks whether a database type name is ゲートキーパー[GPあり].
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isGateKeeperGoalPole(const char* pName) {
    return al::isEqualString(pName, "\u30b2\u30fc\u30c8\u30ad\u30fc\u30d1\u30fc[GP\u3042\u308a]");
}

/**
 * @brief Checks whether a database type name is ゲートキーパー.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isGateKeeperNoGoalPole(const char* pName) {
    return al::isEqualString(pName, "\u30b2\u30fc\u30c8\u30ad\u30fc\u30d1\u30fc");
}

/**
 * @brief Checks whether a database type name is クッパ城.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKoopaCastleNormal(const char* pName) {
    return al::isEqualString(pName, "\u30af\u30c3\u30d1\u57ce");
}

/**
 * @brief Checks whether a database type name is クッパ城[戦車].
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKoopaCastleTank(const char* pName) {
    return al::isEqualString(pName, "\u30af\u30c3\u30d1\u57ce[\u6226\u8eca]");
}

/**
 * @brief Checks whether a database type name is クッパ城[列車].
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKoopaCastleExpress(const char* pName) {
    return al::isEqualString(pName, "\u30af\u30c3\u30d1\u57ce[\u5217\u8eca]");
}

/**
 * @brief Checks whether a database type name is クッパ城[列車通常].
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKoopaCastleExpressNormal(const char* pName) {
    return al::isEqualString(pName, "\u30af\u30c3\u30d1\u57ce[\u5217\u8eca\u901a\u5e38]");
}

/**
 * @brief Checks whether a database type name is クッパ城[砦].
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKoopaCastleFortress(const char* pName) {
    return al::isEqualString(pName, "\u30af\u30c3\u30d1\u57ce[\u7826]");
}

/**
 * @brief Checks whether a database type name is カジノ部屋.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isCasinoRoom(const char* pName) {
    return al::isEqualString(pName, "\u30ab\u30b8\u30ce\u90e8\u5c4b");
}

/**
 * @brief Checks whether a database type name is 妖精の家.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isFairyHouse(const char* pName) {
    return al::isEqualString(pName, "\u5996\u7cbe\u306e\u5bb6");
}

/**
 * @brief Checks whether a database type name is 隠しキノピオの家.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isKinopioHouseHide(const char* pName) {
    return al::isEqualString(pName, "\u96a0\u3057\u30ad\u30ce\u30d4\u30aa\u306e\u5bb6");
}

/**
 * @brief Checks whether a database type name is 隠し土管.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isDokanHide(const char* pName) {
    return al::isEqualString(pName, "\u96a0\u3057\u571f\u7ba1");
}

/**
 * @brief Checks whether a database type name is チャンピオンシップ.
 * @param pName Null-terminated type name to compare.
 * @return True when the names are equal.
 */
bool StageType::isChampionShip(const char* pName) {
    return al::isEqualString(pName, "\u30c1\u30e3\u30f3\u30d4\u30aa\u30f3\u30b7\u30c3\u30d7");
}
