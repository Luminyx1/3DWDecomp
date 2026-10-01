#pragma once

#include <basis/seadTypes.h>
#include <nn/nfp/nfp_types.h>
#include <nn/types.h>

namespace al {
struct NfpCharacterId {
    s16 gameId;
    s16 characterId;
    s16 characterVariant;
};

struct NfpInfo {
    NfpInfo() {}

    void resetError() {
        isError = false;
        isShowError = true;
        _9e = false;
        result = nn::ResultSuccess();
    }

    nn::nfp::TagInfo tagInfo{};
    nn::nfp::ModelInfo modelInfo{};
    nn::Result result;
    bool isError = false;
    bool isShowError = true;
    bool _9e = false;
    u8 _9f;
    u8 _a0[0x9c]{};
    char16 nickName[nn::nfp::AmiiboNameLength + 1]{};
    u8 _152[0x8e]{};
    bool isNormalNfc = false;
    bool isAmiibo = false;
    bool isFormatVersionSet = false;
    bool isNeedRegister = false;
    bool isNeedFormat = false;
    bool isNeedRestore = false;
};

static_assert(sizeof(NfpInfo) == 0x1e8);
}  // namespace al
