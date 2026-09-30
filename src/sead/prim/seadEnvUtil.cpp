#include <prim/seadEnvUtil.h>

#include <nn/oe.h>
#include <nn/settings.h>

#include <container/seadSafeArray.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <filedevice/seadPath.h>
#include <prim/seadStringBuilder.h>

namespace sead
{
static const SafeArray<u8, 32> cRegionAndLanguageTable = {{
    RegionID::JP, LanguageID::ja,  // JPja
    RegionID::US, LanguageID::en,  // USen
    RegionID::US, LanguageID::es,  // USes
    RegionID::US, LanguageID::fr,  // USfr
    RegionID::US, LanguageID::pt,  // USpt
    RegionID::EU, LanguageID::en,  // EUen
    RegionID::EU, LanguageID::es,  // EUes
    RegionID::EU, LanguageID::fr,  // EUfr
    RegionID::EU, LanguageID::de,  // EUde
    RegionID::EU, LanguageID::it,  // EUit
    RegionID::EU, LanguageID::pt,  // EUpt
    RegionID::EU, LanguageID::nl,  // EUnl
    RegionID::EU, LanguageID::ru,  // EUru
    RegionID::KR, LanguageID::ko,  // KRko
    RegionID::CN, LanguageID::zh,  // CNzh
    RegionID::TW, LanguageID::zh,  // TWzh
}};

static const SafeArray<s8, 66> cRegionLanguageTable = {{
    0,  -1, -1, -1, -1, -1,  // ja
    -1, 1,  5,  -1, -1, -1,  // en
    -1, 2,  6,  -1, -1, -1,  // es
    -1, 3,  7,  -1, -1, -1,  // fr
    -1, -1, 8,  -1, -1, -1,  // de
    -1, -1, 9,  -1, -1, -1,  // it
    -1, 4,  10, -1, -1, -1,  // pt
    -1, -1, 11, -1, -1, -1,  // nl
    -1, -1, 12, -1, -1, -1,  // ru
    -1, -1, -1, 13, -1, -1,  // ko
    -1, -1, -1, -1, 14, 15,  // zh
}};

/**
 * Guesses the region from the desired system language.
 * @return the region
 */
static RegionID getDefaultRegion_()
{
    const nn::settings::LanguageCode code = nn::oe::GetDesiredLanguage();
    if (code == nn::settings::Language_Japanese)
    {
        return RegionID::JP;
    }

    if (code == nn::settings::Language_English || code == nn::settings::Language_CanadianFrench ||
        code == nn::settings::Language_LatinAmericanSpanish)
    {
        return RegionID::US;
    }

    if (code == nn::settings::Language_BritishEnglish || code == nn::settings::Language_French ||
        code == nn::settings::Language_German || code == nn::settings::Language_Italian ||
        code == nn::settings::Language_Spanish || code == nn::settings::Language_Dutch ||
        code == nn::settings::Language_Portuguese || code == nn::settings::Language_Russian)
    {
        return RegionID::EU;
    }

    if (code == nn::settings::Language_Korean)
    {
        return RegionID::KR;
    }

    if (code == nn::settings::Language_Chinese ||
        code == nn::settings::Language_SimplifiedChinese)
    {
        return RegionID::CN;
    }

    if (code == nn::settings::Language_Taiwanese ||
        code == nn::settings::Language_TraditionalChinese)
    {
        return RegionID::TW;
    }

    return RegionID::JP;
}

/**
 * Gets the language matching the desired system language.
 * @return the language
 */
static LanguageID getDefaultLanguage_()
{
    const nn::settings::LanguageCode code = nn::oe::GetDesiredLanguage();
    if (code == nn::settings::Language_Japanese)
    {
        return LanguageID::ja;
    }

    if (code == nn::settings::Language_English || code == nn::settings::Language_BritishEnglish)
    {
        return LanguageID::en;
    }

    if (code == nn::settings::Language_French || code == nn::settings::Language_CanadianFrench)
    {
        return LanguageID::fr;
    }

    if (code == nn::settings::Language_German)
    {
        return LanguageID::de;
    }

    if (code == nn::settings::Language_Italian)
    {
        return LanguageID::it;
    }

    if (code == nn::settings::Language_Spanish ||
        code == nn::settings::Language_LatinAmericanSpanish)
    {
        return LanguageID::es;
    }

    if (code == nn::settings::Language_Chinese || code == nn::settings::Language_Taiwanese ||
        code == nn::settings::Language_SimplifiedChinese ||
        code == nn::settings::Language_TraditionalChinese)
    {
        return LanguageID::zh;
    }

    if (code == nn::settings::Language_Korean)
    {
        return LanguageID::ko;
    }

    if (code == nn::settings::Language_Dutch)
    {
        return LanguageID::nl;
    }

    if (code == nn::settings::Language_Portuguese)
    {
        return LanguageID::pt;
    }

    if (code == nn::settings::Language_Russian)
    {
        return LanguageID::ru;
    }

    return LanguageID::ja;
}

SEAD_SINGLETON_DISPOSER_IMPL(RegionLanguageMgr)

/**
 * Creates an uninitialized manager.
 */
RegionLanguageMgr::RegionLanguageMgr() : mMaskBuffer(nullptr), mIsInitialized(false) {}

/**
 * Frees the mask buffer.
 */
RegionLanguageMgr::~RegionLanguageMgr()
{
    delete[] mMaskBuffer;
}

/**
 * Parses a space separated list of region languages.
 * @param pMask receives the parsed region languages (cleared on failure)
 * @param rMaskStr list to parse
 * @return whether at least one region language was parsed and all items were valid
 */
bool RegionLanguageMgr::parseRegionLanguageMaskStr_(RingBuffer<RegionLanguageID>* pMask,
                                                    const SafeString& rMaskStr) const
{
    pMask->clear();
    FixedSafeString<16> token;
    const SafeString delimiter = " ";
    auto it = rMaskStr.tokenBegin(delimiter);
    const auto end = rMaskStr.tokenEnd(delimiter);
    while (end != it)
    {
        if (it.getAndForward(&token) != 4)
        {
            pMask->clear();
            return false;
        }

        RegionLanguageID regionLanguage;
        if (!EnvUtil::getRegionLanguageFromString(&regionLanguage, token))
        {
            pMask->clear();
            return false;
        }

        pMask->pushBack(regionLanguage);
    }

    if (pMask->empty())
    {
        pMask->clear();
        return false;
    }

    return true;
}

/**
 * Gets a region language from its name.
 * @param pRegionLanguage receives the region language
 * @param rStr name of the region language
 * @return whether the name is valid
 */
bool EnvUtil::getRegionLanguageFromString(RegionLanguageID* pRegionLanguage,
                                          const SafeString& rStr)
{
    for (auto it = RegionLanguageID::begin(); it != RegionLanguageID::end(); ++it)
    {
        if (SafeString((*it).text()) == rStr)
        {
            *pRegionLanguage = *it;
            return true;
        }
    }

    return false;
}

// NON_MATCHING: ~87% (with RingBuffer::front checking the size); loop registers differ
void RegionLanguageMgr::setRegionLanguageWithCheckMask_(RegionID region, LanguageID language,
                                                        const RingBuffer<RegionLanguageID>& rMask)
{
    RegionLanguageID regionLanguage;
    if (EnvUtil::convertToRegionLanguage(&regionLanguage, region, language))
    {
        for (auto it = rMask.begin(); it != rMask.end(); ++it)
        {
            if (static_cast<s32>(regionLanguage) == static_cast<s32>(*it))
            {
                mRegionLanguage = regionLanguage;
                return;
            }
        }
    }

    for (auto it = rMask.begin(); it != rMask.end(); ++it)
    {
        RegionID maskRegion;
        LanguageID maskLanguage;
        EnvUtil::convertToRegionAndLanguage(&maskRegion, &maskLanguage, *it);
        if (static_cast<s32>(region) == static_cast<s32>(maskRegion))
        {
            mRegionLanguage = *it;
            return;
        }
    }

    mRegionLanguage = rMask.front();
}

/**
 * Combines a region and a language.
 * @param pRegionLanguage receives the region language
 * @param region region
 * @param language language
 * @return whether the combination exists
 */
bool EnvUtil::convertToRegionLanguage(RegionLanguageID* pRegionLanguage, RegionID region,
                                      LanguageID language)
{
    const s32 regionLanguage = cRegionLanguageTable[region + language * RegionID::size()];
    if (regionLanguage < 0)
    {
        return false;
    }

    *pRegionLanguage = regionLanguage;
    return true;
}

/**
 * Splits a region language into its region and language.
 * @param pRegion receives the region
 * @param pLanguage receives the language
 * @param regionLanguage region language to split
 */
void EnvUtil::convertToRegionAndLanguage(RegionID* pRegion, LanguageID* pLanguage,
                                         RegionLanguageID regionLanguage)
{
    *pRegion = cRegionAndLanguageTable[regionLanguage.getRelativeIndex() * 2];
    *pLanguage = cRegionAndLanguageTable[regionLanguage.getRelativeIndex() * 2 + 1];
}

/**
 * Selects the region language from the system language and the allowed region languages.
 * @param rArg mask string, mask file and rom type
 */
void RegionLanguageMgr::initialize(const InitArg& rArg)
{
    if (mIsInitialized)
    {
        return;
    }

    FixedRingBuffer<RegionLanguageID, RegionLanguageID::size()> mask;
    if (!parseRegionLanguageMaskStr_(&mask, rArg.maskString))
    {
        for (auto it = RegionLanguageID::begin(); it != RegionLanguageID::end(); ++it)
        {
            mask.pushBack(*it);
        }
    }

    mRomType = rArg.romType;
    if (rArg.maskFilePath)
    {
        loadMask_(&mask, rArg);
    }

    setRegionLanguageWithCheckMask_(getDefaultRegion_(), getDefaultLanguage_(), mask);
    mIsInitialized = true;
}

/**
 * Initializes the manager with a fixed region language.
 * @param regionLanguage region language to use
 */
void RegionLanguageMgr::initializeWithRegionLanguage(RegionLanguageID regionLanguage,
                                                     Controller*)
{
    if (mIsInitialized)
    {
        return;
    }

    mRegionLanguage = regionLanguage;
    mIsInitialized = true;
}

/**
 * Gets the current region.
 * @return the region
 */
RegionID RegionLanguageMgr::getRegion() const
{
    return cRegionAndLanguageTable[mRegionLanguage.getRelativeIndex() * 2];
}

/**
 * Gets the current language.
 * @return the language
 */
LanguageID RegionLanguageMgr::getLanguage() const
{
    return cRegionAndLanguageTable[mRegionLanguage.getRelativeIndex() * 2 + 1];
}

/**
 * Sets the current region language.
 * @param regionLanguage region language to use
 */
void RegionLanguageMgr::setRegionLanguage(RegionLanguageID regionLanguage)
{
    mRegionLanguage = regionLanguage;
}

/**
 * Sets the current region language from a region and a language.
 * @param region region
 * @param language language
 * @return whether the combination exists
 */
bool RegionLanguageMgr::setRegionAndLanguage(RegionID region, LanguageID language)
{
    return EnvUtil::convertToRegionLanguage(&mRegionLanguage, region, language);
}

/**
 * Gets the platform identifier.
 * @return the platform
 */
s32 EnvUtil::getPlatform()
{
    return 4;
}

/**
 * Gets the build platform identifier.
 * @return the build platform
 */
s32 EnvUtil::getBuildPlatform()
{
    return 6;
}

/**
 * Gets the build target identifier.
 * @return the target
 */
s32 EnvUtil::getTarget()
{
    return 3;
}

/**
 * Gets the current region, falling back to the system settings.
 * @return the region
 */
RegionID EnvUtil::getRegion()
{
    RegionLanguageMgr* mgr = RegionLanguageMgr::instance();
    if (mgr && mgr->isInitialized())
    {
        return mgr->getRegion();
    }

    return getDefaultRegion_();
}

/**
 * Gets the current language, falling back to the system settings.
 * @return the language
 */
LanguageID EnvUtil::getLanguage()
{
    RegionLanguageMgr* mgr = RegionLanguageMgr::instance();
    if (mgr && mgr->isInitialized())
    {
        return mgr->getLanguage();
    }

    return getDefaultLanguage_();
}

/**
 * Gets the main language of a region.
 * @param region region
 * @return the language
 */
static LanguageID getDefaultLanguageOfRegion_(RegionID region)
{
    switch (region)
    {
    case RegionID::US:
    case RegionID::EU:
        return LanguageID::en;
    case RegionID::KR:
        return LanguageID::ko;
    case RegionID::CN:
    case RegionID::TW:
        return LanguageID::zh;
    default:
        return LanguageID::ja;
    }
}

// NON_MATCHING: ~98%; the fallback clamps the region language in 8 bits
RegionLanguageID EnvUtil::getRegionLanguage()
{
    RegionLanguageMgr* mgr = RegionLanguageMgr::instance();
    if (mgr && mgr->isInitialized())
    {
        return mgr->getRegionLanguage();
    }

    const RegionID region = getDefaultRegion_();
    RegionLanguageID regionLanguage;
    if (convertToRegionLanguage(&regionLanguage, region, getDefaultLanguage_()))
    {
        return regionLanguage;
    }

    convertToRegionLanguage(&regionLanguage, region, getDefaultLanguageOfRegion_(region));
    return regionLanguage;
}

/**
 * Gets the rom type set at initialization.
 * @return the rom type, or an empty string
 */
const SafeString& EnvUtil::getRomType()
{
    RegionLanguageMgr* mgr = RegionLanguageMgr::instance();
    if (mgr && mgr->isInitialized())
    {
        return mgr->getRomType();
    }

    return SafeString::cEmptyString;
}

/**
 * Gets a region from its name.
 * @param pRegion receives the region
 * @param rStr name of the region
 * @return whether the name is valid
 */
bool EnvUtil::getRegionFromString(RegionID* pRegion, const SafeString& rStr)
{
    return pRegion->fromText(rStr);
}

/**
 * Gets a language from its name.
 * @param pLanguage receives the language
 * @param rStr name of the language
 * @return whether the name is valid
 */
bool EnvUtil::getLanguageFromString(LanguageID* pLanguage, const SafeString& rStr)
{
    return pLanguage->fromText(rStr);
}

/**
 * Prints the current time (no-op in release builds).
 */
void EnvUtil::printCurrentTime() {}

/**
 * Gets an environment variable (unsupported).
 * @return -1
 */
s32 EnvUtil::getEnvironmentVariable(BufferedSafeString*, const SafeString&)
{
    return -1;
}

/**
 * Gets the computer name (unsupported).
 * @return -1
 */
s32 EnvUtil::getComputerName(BufferedSafeString*)
{
    return -1;
}

/**
 * Converts a path to a Windows path (empty in release builds).
 * @param pOut receives the path
 * @param rPath path to convert
 * @return 0
 */
s32 EnvUtil::convertToWinPath(BufferedSafeString* pOut, const SafeString& rPath)
{
    pOut->clear();
    FileDeviceMgr* mgr = FileDeviceMgr::instance();
    if (!mgr)
    {
        return 0;
    }

    {
        FixedSafeString<64> drive;
        if (Path::getDriveName(&drive, rPath))
        {
            if (!mgr->findDevice(drive))
            {
                return 0;
            }

            mgr->resolveFilePath(pOut, rPath);
        }
        else
        {
            mgr->getDefaultFileDevice()->resolveFilePath(pOut, rPath);
        }
    }

    FixedSafeString<512> path;
    path.clear();
    pOut->clear();
    return 0;
}

/**
 * Resolves environment variables in a string (unsupported).
 * @param pOut receives the resolved string (cleared)
 * @return 0
 */
s32 EnvUtil::resolveEnvronmentVariable(BufferedSafeString* pOut, const SafeString&)
{
    pOut->clear();
    return 0;
}

/**
 * Gets a full path (unsupported).
 * @param pOut receives the path (cleared)
 * @return 0
 */
s32 EnvUtil::getFullPath(BufferedSafeString* pOut, const SafeString&)
{
    pOut->clear();
    return 0;
}
}  // namespace sead
