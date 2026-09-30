#include "Library/Nfp/NfpFunction.hpp"

#include "Library/Nfp/NfpTypes.hpp"

namespace al {
/**
 * Checks whether any NFC tag was detected.
 * @param rInfo NFP info
 * @return whether an amiibo or a normal NFC tag was detected
 */
bool isAnyNfcTagDetected(const NfpInfo& rInfo) {
    return rInfo.isAmiibo || rInfo.isNormalNfc;
}

/**
 * Checks whether a normal NFC tag was detected.
 * @param rInfo NFP info
 * @return whether it is a normal NFC tag
 */
bool isNormalNfc(const NfpInfo& rInfo) {
    return rInfo.isNormalNfc;
}

/**
 * Checks whether an amiibo was detected.
 * @param rInfo NFP info
 * @return whether it is an amiibo
 */
bool isAmiibo(const NfpInfo& rInfo) {
    return rInfo.isAmiibo;
}

/**
 * Checks whether two amiibo have the same unique id.
 * @param rInfoA first NFP info
 * @param rInfoB second NFP info
 * @return whether both are amiibo with the same id
 */
bool isEqualUniqueNfcId(const NfpInfo& rInfoA, const NfpInfo& rInfoB) {
    if (!rInfoA.isAmiibo || !rInfoB.isAmiibo) {
        return false;
    }
    return isEqualUniqueNfcId(rInfoA.tagInfo, rInfoB.tagInfo);
}

/**
 * Checks whether two tags have the same unique id.
 * @param rTagInfoA first tag info
 * @param rTagInfoB second tag info
 * @return whether the ids are the same
 */
bool isEqualUniqueNfcId(const nn::nfp::TagInfo& rTagInfoA, const nn::nfp::TagInfo& rTagInfoB) {
    return isEqualUniqueNfcId(rTagInfoA.id, rTagInfoB.id);
}

/**
 * Checks whether an amiibo is a character variant.
 * @param rInfo NFP info
 * @param characterId character id
 * @return whether it is an amiibo of the character variant
 */
bool isEqualCharacterId(const NfpInfo& rInfo, NfpCharacterId characterId) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    return isEqualCharacterId(rInfo.modelInfo, characterId);
}

/**
 * Checks whether a model is a character variant.
 * @param rModelInfo model info
 * @param rCharacterId character id
 * @return whether the character and its variant are the same
 */
bool isEqualCharacterId(const nn::nfp::ModelInfo& rModelInfo, const NfpCharacterId& rCharacterId) {
    return isEqualCharacterIdBase(rModelInfo, rCharacterId) &&
           rCharacterId.characterVariant == rModelInfo.characterVariant;
}

/**
 * Checks whether an amiibo is a character, ignoring the variant.
 * @param rInfo NFP info
 * @param characterId character id
 * @return whether it is an amiibo of the character
 */
bool isEqualCharacterIdBase(const NfpInfo& rInfo, NfpCharacterId characterId) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    return isEqualCharacterIdBase(rInfo.modelInfo, characterId);
}

/**
 * Checks whether a model is a character, ignoring the variant.
 * @param rModelInfo model info
 * @param rCharacterId character id
 * @return whether the character is the same
 */
bool isEqualCharacterIdBase(const nn::nfp::ModelInfo& rModelInfo,
                            const NfpCharacterId& rCharacterId) {
    if (rCharacterId.gameId != rModelInfo.gameId) {
        return false;
    }
    return rCharacterId.characterId == rModelInfo.characterId;
}

/**
 * Checks the numbering id of an amiibo.
 * @param rInfo NFP info
 * @param numberingId numbering id
 * @return whether it is an amiibo with the numbering id
 */
bool isEqualNumberingId(const NfpInfo& rInfo, s32 numberingId) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    return isEqualNumberingId(rInfo.modelInfo, numberingId);
}

/**
 * Checks the numbering id of a model.
 * @param rModelInfo model info
 * @param numberingId numbering id
 * @return whether the numbering id is the same
 */
bool isEqualNumberingId(const nn::nfp::ModelInfo& rModelInfo, s32 numberingId) {
    return rModelInfo.modelNumber == numberingId;
}

/**
 * Gets the character id of an amiibo.
 * @param pCharacterId character id
 * @param rInfo NFP info
 * @return whether it is an amiibo
 */
bool tryGetCharacterId(NfpCharacterId* pCharacterId, const NfpInfo& rInfo) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    pCharacterId->gameId = rInfo.modelInfo.gameId;
    pCharacterId->characterId = rInfo.modelInfo.characterId;
    pCharacterId->characterVariant = rInfo.modelInfo.characterVariant;
    return true;
}

/**
 * Gets the numbering id of an amiibo.
 * @param pNumberingId numbering id
 * @param rInfo NFP info
 * @return whether it is an amiibo
 */
bool tryGetNumberingId(s32* pNumberingId, const NfpInfo& rInfo) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    *pNumberingId = rInfo.modelInfo.modelNumber;
    return true;
}

/**
 * Checks the series id of an amiibo.
 * @param rInfo NFP info
 * @param seriesId series id
 * @return whether it is an amiibo of the series
 */
bool isEqualSeriesID(const NfpInfo& rInfo, s32 seriesId) {
    return rInfo.isAmiibo & isEqualSeriesID(rInfo.modelInfo, seriesId);
}

/**
 * Checks the series id of a model.
 * @param rModelInfo model info
 * @param seriesId series id
 * @return whether the series id is the same
 */
bool isEqualSeriesID(const nn::nfp::ModelInfo& rModelInfo, s32 seriesId) {
    return rModelInfo.amiiboType == seriesId;
}

/**
 * Gets the series id of an amiibo.
 * @param pSeriesId series id
 * @param rInfo NFP info
 * @return whether it is an amiibo
 */
bool tryGetSeriesID(s32* pSeriesId, const NfpInfo& rInfo) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    *pSeriesId = rInfo.modelInfo.amiiboType;
    return true;
}

/**
 * Gets the type of an amiibo.
 * @param pNfpType amiibo type
 * @param rInfo NFP info
 * @return whether it is an amiibo
 */
bool tryGetNfpType(s32* pNfpType, const NfpInfo& rInfo) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    *pNfpType = rInfo.modelInfo.series;
    return true;
}

/**
 * Checks whether an amiibo is Mario.
 * @param rInfo NFP info
 * @return whether it is a Mario amiibo
 */
bool isCharacterIdBaseMario(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x00, 0x00});
}

/**
 * Checks whether an amiibo is Dr. Mario.
 * @param rInfo NFP info
 * @return whether it is a Dr. Mario amiibo
 */
bool isCharacterIdBaseDrMario(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x00, 0x01});
}

/**
 * Checks whether an amiibo is Peach.
 * @param rInfo NFP info
 * @return whether it is a Peach amiibo
 */
bool isCharacterIdBasePeach(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x02, 0x00});
}

/**
 * Checks whether an amiibo is Bowser.
 * @param rInfo NFP info
 * @return whether it is a Bowser amiibo
 */
bool isCharacterIdBaseKoopa(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x05, 0x00});
}

/**
 * Checks whether an amiibo is Bowser Jr.
 * @param rInfo NFP info
 * @return whether it is a Bowser Jr. amiibo
 */
bool isCharacterIdBaseKoopaJr(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x06, 0x00});
}

/**
 * Checks whether an amiibo is Yoshi.
 * @param rInfo NFP info
 * @return whether it is a Yoshi amiibo
 */
bool isCharacterIdBaseYoshi(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x03, 0x00});
}

/**
 * Checks whether an amiibo is Toad.
 * @param rInfo NFP info
 * @return whether it is a Toad amiibo
 */
bool isCharacterIdBaseKinopio(const NfpInfo& rInfo) {
    return isEqualCharacterIdBase(rInfo, {0x00, 0x0a, 0x00});
}

/**
 * Checks whether the tag needs to be formatted.
 * @param rInfo NFP info
 * @return whether it needs to be formatted
 */
bool isNeedFormat(const NfpInfo& rInfo) {
    return rInfo.isNeedFormat;
}

/**
 * Checks whether the tag needs to be restored.
 * @param rInfo NFP info
 * @return whether it needs to be restored
 */
bool isNeedRestore(const NfpInfo& rInfo) {
    return rInfo.isNeedRestore;
}

/**
 * Checks whether the amiibo needs to be registered.
 * @param rInfo NFP info
 * @return whether it needs to be registered
 */
bool isNeedRegister(const NfpInfo& rInfo) {
    return rInfo.isNeedRegister;
}

/**
 * Checks whether the amiibo has no nickname.
 * @param rInfo NFP info
 * @return whether the nickname is empty
 */
bool isNeedRegisterNickName(const NfpInfo& rInfo) {
    return static_cast<char>(rInfo.nickName[0]) == '\0';
}

/**
 * Checks whether the register info is valid.
 * @param rInfo NFP info
 * @return always false
 */
bool isValidRegisterInfo(const NfpInfo& rInfo) {
    return false;
}

/**
 * Does nothing.
 * @param pOwnerName owner name
 * @param rInfo NFP info
 */
void getAmiiboOwnerName(sead::BufferedSafeStringBase<char16>* pOwnerName, const NfpInfo& rInfo) {}

/**
 * Gets the nickname of an amiibo.
 * @param pNickName nickname
 * @param rInfo NFP info
 */
void getAmiiboNickName(sead::BufferedSafeStringBase<char16>* pNickName, const NfpInfo& rInfo) {
    pNickName->copy(rInfo.nickName);
}

/**
 * Checks whether the format version of the tag is unsupported.
 * @param rInfo NFP info
 * @return whether the format version is invalid
 */
bool isInvalidFormatVersion(const NfpInfo& rInfo) {
    if (!rInfo.isFormatVersionSet) {
        return false;
    }
    if (rInfo.formatVersion.minor != 0x73) {
        return false;
    }
    return rInfo.formatVersion.major == 0xb8;
}

/**
 * Checks whether a tag and an amiibo have the same unique id.
 * @param rTagInfo tag info
 * @param rInfo NFP info
 * @return whether the NFP info is an amiibo with the same id
 */
bool isEqualUniqueNfcId(const nn::nfp::TagInfo& rTagInfo, const NfpInfo& rInfo) {
    if (!rInfo.isAmiibo) {
        return false;
    }
    return isEqualUniqueNfcId(rTagInfo, rInfo.tagInfo);
}

/**
 * Checks whether two tag ids are the same.
 * @param rTagIdA first tag id
 * @param rTagIdB second tag id
 * @return whether the ids are the same
 */
bool isEqualUniqueNfcId(const nn::nfp::TagId& rTagIdA, const nn::nfp::TagId& rTagIdB) {
    if (rTagIdA.uuidLength != rTagIdB.uuidLength) {
        return false;
    }
    for (s32 i = 0; i < rTagIdA.uuidLength; i++) {
        if (rTagIdA.uuid[i] != rTagIdB.uuid[i]) {
            return false;
        }
    }
    return true;
}
}  // namespace al
