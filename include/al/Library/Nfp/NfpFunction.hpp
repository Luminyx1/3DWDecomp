#pragma once

#include <prim/seadSafeString.h>

namespace nn::nfp {
struct ModelInfo;
struct TagId;
struct TagInfo;
}  // namespace nn::nfp

namespace al {
struct NfpCharacterId;
struct NfpInfo;

bool isAnyNfcTagDetected(const NfpInfo& rInfo);
bool isNormalNfc(const NfpInfo& rInfo);
bool isAmiibo(const NfpInfo& rInfo);
bool isEqualUniqueNfcId(const NfpInfo& rInfoA, const NfpInfo& rInfoB);
bool isEqualUniqueNfcId(const nn::nfp::TagInfo& rTagInfoA, const nn::nfp::TagInfo& rTagInfoB);
bool isEqualCharacterId(const NfpInfo& rInfo, NfpCharacterId characterId);
bool isEqualCharacterId(const nn::nfp::ModelInfo& rModelInfo, const NfpCharacterId& rCharacterId);
bool isEqualCharacterIdBase(const NfpInfo& rInfo, NfpCharacterId characterId);
bool isEqualCharacterIdBase(const nn::nfp::ModelInfo& rModelInfo,
                            const NfpCharacterId& rCharacterId);
bool isEqualNumberingId(const NfpInfo& rInfo, s32 numberingId);
bool isEqualNumberingId(const nn::nfp::ModelInfo& rModelInfo, s32 numberingId);
bool tryGetCharacterId(NfpCharacterId* pCharacterId, const NfpInfo& rInfo);
bool tryGetNumberingId(s32* pNumberingId, const NfpInfo& rInfo);
bool isEqualSeriesID(const NfpInfo& rInfo, s32 seriesId);
bool isEqualSeriesID(const nn::nfp::ModelInfo& rModelInfo, s32 seriesId);
bool tryGetSeriesID(s32* pSeriesId, const NfpInfo& rInfo);
bool tryGetNfpType(s32* pNfpType, const NfpInfo& rInfo);
bool isCharacterIdBaseMario(const NfpInfo& rInfo);
bool isCharacterIdBaseDrMario(const NfpInfo& rInfo);
bool isCharacterIdBasePeach(const NfpInfo& rInfo);
bool isCharacterIdBaseKoopa(const NfpInfo& rInfo);
bool isCharacterIdBaseKoopaJr(const NfpInfo& rInfo);
bool isCharacterIdBaseYoshi(const NfpInfo& rInfo);
bool isCharacterIdBaseKinopio(const NfpInfo& rInfo);
bool isNeedFormat(const NfpInfo& rInfo);
bool isNeedRestore(const NfpInfo& rInfo);
bool isNeedRegister(const NfpInfo& rInfo);
bool isNeedRegisterNickName(const NfpInfo& rInfo);
bool isValidRegisterInfo(const NfpInfo& rInfo);
void getAmiiboOwnerName(sead::BufferedSafeStringBase<char16>* pOwnerName, const NfpInfo& rInfo);
void getAmiiboNickName(sead::BufferedSafeStringBase<char16>* pNickName, const NfpInfo& rInfo);
bool isInvalidFormatVersion(const NfpInfo& rInfo);
bool isEqualUniqueNfcId(const nn::nfp::TagInfo& rTagInfo, const NfpInfo& rInfo);
bool isEqualUniqueNfcId(const nn::nfp::TagId& rTagIdA, const nn::nfp::TagId& rTagIdB);
}  // namespace al
