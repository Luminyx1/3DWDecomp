#pragma once

#include <container/seadRingBuffer.h>
#include <heap/seadDisposer.h>
#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Controller;
class Heap;

SEAD_ENUM(RegionLanguageID, JPja, USen, USes, USfr, USpt, EUen, EUes, EUfr, EUde, EUit, EUpt, EUnl, EUru, KRko, CNzh, TWzh)
SEAD_ENUM(RegionID, JP, US, EU, KR, CN, TW)
SEAD_ENUM(LanguageID, ja, en, es, fr, de, it, pt, nl, ru, ko, zh)

class RegionLanguageMgr
{
    SEAD_SINGLETON_DISPOSER(RegionLanguageMgr)

public:
    struct InitArg
    {
        const char* maskString;
        SafeString romType;
        const char* maskFilePath;
        const char* maskFileName;
        Heap* heap;
    };

    static const char* cRegionLanguageMaskStr_All;

    RegionLanguageMgr();
    ~RegionLanguageMgr();

    void initialize(const InitArg& rArg);
    void initializeWithRegionLanguage(RegionLanguageID regionLanguage, Controller* pController);

    RegionID getRegion() const;
    LanguageID getLanguage() const;
    void setRegionLanguage(RegionLanguageID regionLanguage);
    bool setRegionAndLanguage(RegionID region, LanguageID language);

    RegionLanguageID getRegionLanguage() const { return mRegionLanguage; }
    const SafeString& getRomType() const { return mRomType; }
    bool isInitialized() const { return mIsInitialized; }

private:
    void loadMask_(RingBuffer<RegionLanguageID>* pMask, const InitArg& rArg);
    bool parseRegionLanguageMaskStr_(RingBuffer<RegionLanguageID>* pMask,
                                     const SafeString& rMaskStr) const;
    void setRegionLanguageWithCheckMask_(RegionID region, LanguageID language,
                                         const RingBuffer<RegionLanguageID>& rMask);

    RegionLanguageID mRegionLanguage;
    SafeString mRomType;
    char* mMaskBuffer;
    bool mIsInitialized;
};

class EnvUtil
{
public:
    static s32 getPlatform();
    static s32 getBuildPlatform();
    static s32 getTarget();
    static RegionID getRegion();
    static LanguageID getLanguage();
    static RegionLanguageID getRegionLanguage();
    static const SafeString& getRomType();

    static bool getRegionFromString(RegionID* pRegion, const SafeString& rStr);
    static bool getLanguageFromString(LanguageID* pLanguage, const SafeString& rStr);
    static bool getRegionLanguageFromString(RegionLanguageID* pRegionLanguage,
                                            const SafeString& rStr);
    static bool convertToRegionLanguage(RegionLanguageID* pRegionLanguage, RegionID region,
                                        LanguageID language);
    static void convertToRegionAndLanguage(RegionID* pRegion, LanguageID* pLanguage,
                                           RegionLanguageID regionLanguage);

    static void printCurrentTime();
    static s32 getEnvironmentVariable(BufferedSafeString* pOut, const SafeString& rVariable);
    static s32 getComputerName(BufferedSafeString* pOut);
    static s32 convertToWinPath(BufferedSafeString* pOut, const SafeString& rPath);
    static s32 resolveEnvronmentVariable(BufferedSafeString* pOut, const SafeString& rStr);
    static s32 getFullPath(BufferedSafeString* pOut, const SafeString& rPath);
};
}  // namespace sead
