#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;
class IAudioResourceLoader;
class Resource;
class SeadAudioPlayer;
class SeResourceSpecificInfo;
class SeUserInfo;

class SeBankLoadingInfo {
public:
    static SeBankLoadingInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeBankLoadingInfo* pA, const SeBankLoadingInfo* pB);

    const char* mName;
    u32 mSoundId;
};

static_assert(sizeof(SeBankLoadingInfo) == 0x10);

class SeUserLoadingInfo {
public:
    static SeUserLoadingInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeUserLoadingInfo* pA, const SeUserLoadingInfo* pB);

    const char* mName;
};

static_assert(sizeof(SeUserLoadingInfo) == 0x8);

class SeArchiveLoadingInfo {
public:
    static SeArchiveLoadingInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeArchiveLoadingInfo* pA, const SeArchiveLoadingInfo* pB);

    const char* mName;
    AudioInfoList<SeBankLoadingInfo>* mBankLoadingInfoList;
    AudioInfoList<SeUserLoadingInfo>* mUserLoadingInfoList;
};

static_assert(sizeof(SeArchiveLoadingInfo) == 0x18);

class SeDataBase {
public:
    static const char* ARC_NAME;

    SeDataBase(const char* pArchiveName, SeadAudioPlayer* pPlayer, const char* pUnused);

    void createUserInfoList(const Resource* pResource);
    void createResourceSespecificInfoList(const Resource* pResource, SeadAudioPlayer* pPlayer);
    void createStationedArchiveInfoList(const Resource* pResource);
    void setResourceSpecInfoToResourceInfo();
    void callLoadResourceStationedSound(IAudioResourceLoader* pLoader) const;
    void callLoadBankListBank(IAudioResourceLoader* pLoader, const sead::SafeString& rName) const;
    void requestLoadResourceStationedSound() const;
    void requestLoadResourceStationedObjectSound() const;
    void requestLoadBankListBank(const sead::SafeString& rName) const;

    AudioInfoList<SeUserInfo>* getUserInfoList() const { return mUserInfoList; }
    AudioInfoList<SeResourceSpecificInfo>* getResourceSpecificInfoList() const { return mResourceSpecificInfoList; }

    const SeResourceSpecificInfo* tryFindResourceSpecificInfo(const char* pName) const {
        if (mResourceSpecificInfoList == nullptr) {
            return nullptr;
        }

        if (pName == nullptr) {
            return nullptr;
        }

        return mResourceSpecificInfoList->tryFindInfo(pName);
    }

private:
    AudioInfoList<SeUserInfo>* mUserInfoList = nullptr;
    AudioInfoList<SeResourceSpecificInfo>* mResourceSpecificInfoList = nullptr;
    AudioInfoList<SeArchiveLoadingInfo>* mArchiveLoadingInfoList = nullptr;
};

static_assert(sizeof(SeDataBase) == 0x18);
}  // namespace al
