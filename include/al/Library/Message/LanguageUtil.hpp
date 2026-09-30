#pragma once

enum alLanguage {
    alLanguage_JPja,
    alLanguage_USen,
    alLanguage_EUfr,
    alLanguage_EUde,
    alLanguage_EUit,
    alLanguage_EUes,
    alLanguage_CNzh,
    alLanguage_KRko,
    alLanguage_EUnl,
    alLanguage_EUpt,
    alLanguage_EUru,
    alLanguage_TWzh,
    alLanguage_EUen,
    alLanguage_USfr,
    alLanguage_USes,
    alLanguage_Invalid,
};

namespace al {
void initRegionAndLanguage();
void forceInitLanguage(alLanguage language);
const char* getLanguageString();
void forceInitLanguage(const char* pLanguage);
bool isLanguageEmQuad();
alLanguage getLanguageCode();
}  // namespace al
