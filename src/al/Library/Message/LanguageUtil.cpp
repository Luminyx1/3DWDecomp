#include "Library/Message/LanguageUtil.hpp"

#include <prim/seadEnvUtil.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
struct LanguageInfo {
    alLanguage language;
    const char* name;
    const char* directoryName;
};

const LanguageInfo cLanguageInfos[] = {
    {alLanguage_JPja, "JPja", "JpJa"}, {alLanguage_USen, "USen", "UsEn"},
    {alLanguage_USes, "USes", "UsEs"}, {alLanguage_USfr, "USfr", "UsFr"},
    {alLanguage_EUen, "EUen", "EuEn"}, {alLanguage_EUes, "EUes", "EuEs"},
    {alLanguage_EUfr, "EUfr", "EuFr"}, {alLanguage_EUde, "EUde", "EuDe"},
    {alLanguage_EUit, "EUit", "EuIt"}, {alLanguage_EUpt, "EUpt", "EuPt"},
    {alLanguage_EUnl, "EUnl", "EuNl"}, {alLanguage_EUru, "EUru", "EuRu"},
    {alLanguage_KRko, "KRko", "KRko"}, {alLanguage_CNzh, "CNzh", "CNzh"},
    {alLanguage_TWzh, "TWzh", "TWzh"},
};

alLanguage sLanguage = alLanguage_Invalid;

constexpr s32 cLanguageInfoNum = sizeof(cLanguageInfos) / sizeof(cLanguageInfos[0]);

alLanguage getLanguageFromString(const char* pName) {
    s32 index = -1;

    for (s32 i = 0; i < cLanguageInfoNum; i++) {
        if (isEqualString(pName, cLanguageInfos[i].name)) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return alLanguage_JPja;
    }

    return cLanguageInfos[index].language;
}

inline const char* findLanguageDirectoryName(alLanguage language) {
    s32 index = -1;

    for (s32 i = 0; i < cLanguageInfoNum; i++) {
        if (cLanguageInfos[i].language == language) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return "JpJa";
    }

    return cLanguageInfos[index].directoryName;
}
}  // namespace

/**
 * Initializes the region language manager and the current language.
 */
void initRegionAndLanguage() {
    sead::RegionLanguageMgr* mgr = sead::RegionLanguageMgr::createInstance(nullptr);
    sead::RegionLanguageMgr::InitArg arg = {sead::RegionLanguageMgr::cRegionLanguageMaskStr_All,
                                            sead::SafeString::cEmptyString, nullptr, nullptr,
                                            nullptr};
    mgr->initialize(arg);
    sLanguage = getLanguageFromString(mgr->getRegionLanguage().text());
}

/**
 * Forces the current language.
 * @param language language to use
 */
void forceInitLanguage(alLanguage language) {
    sead::RegionLanguageMgr* mgr = sead::RegionLanguageMgr::instance();
    sLanguage = language;
    sead::RegionLanguageID regionLanguage;
    sead::EnvUtil::getRegionLanguageFromString(&regionLanguage, getLanguageString());
    mgr->setRegionLanguage(regionLanguage);
}

/**
 * Returns the directory name of the current language.
 * @return language directory name
 */
const char* getLanguageString() {
    return findLanguageDirectoryName(sLanguage);
}

/**
 * Forces the current language by name.
 * @param pLanguage language name, such as "USen"
 */
void forceInitLanguage(const char* pLanguage) {
    forceInitLanguage(getLanguageFromString(pLanguage));
}

/**
 * Checks whether the current language uses full width spaces.
 * @return whether the language is Japanese, Chinese or Korean
 */
bool isLanguageEmQuad() {
    switch (sLanguage) {
    case alLanguage_JPja:
    case alLanguage_CNzh:
    case alLanguage_KRko:
    case alLanguage_TWzh:
        return true;
    default:
        return false;
    }
}

/**
 * Returns the current language.
 * @return language
 */
alLanguage getLanguageCode() {
    return sLanguage;
}
}  // namespace al
