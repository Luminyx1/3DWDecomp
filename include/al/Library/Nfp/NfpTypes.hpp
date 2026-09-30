#pragma once

#include <basis/seadTypes.h>
#include <nn/nfp/nfp_types.h>

namespace al {
struct NfpCharacterId {
    s16 gameId;
    s16 characterId;
    s16 characterVariant;
};

struct NfpFormatVersion {
    u32 minor : 9;
    u32 major : 13;
};

struct NfpInfo {
    nn::nfp::TagInfo tagInfo;
    nn::nfp::ModelInfo modelInfo;
    NfpFormatVersion formatVersion;
    u8 _9c[0xa0];
    char16 nickName[nn::nfp::AmiiboNameLength + 1];
    u8 _152[0x8e];
    bool isNormalNfc;
    bool isAmiibo;
    bool isFormatVersionSet;
    bool isNeedRegister;
    bool isNeedFormat;
    bool isNeedRestore;
};
}  // namespace al
