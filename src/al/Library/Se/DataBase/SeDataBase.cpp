#include "Library/Se/DataBase/SeDataBase.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
SeResourceSpecificInfo gDefaultSeResourceSpecificInfo;

/**
 * Loads the SE database from the given archive.
 * @param pArchiveName Archive path.
 * @param pPlayer Audio player used to read sound information.
 * @param pUnused Unused.
 */
SeDataBase::SeDataBase(const char* pArchiveName, SeadAudioPlayer* pPlayer, const char* pUnused) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    createUserInfoList(resource);
    createResourceSespecificInfoList(resource, pPlayer);
    createStationedArchiveInfoList(resource);
    setResourceSpecInfoToResourceInfo();
}

/**
 * Creates the SE user information of all user files in the resource.
 * @param pResource SE database resource.
 */
void SeDataBase::createUserInfoList(const Resource* pResource) {
    s32 entryNum = pResource->getEntryNum("/");
    AudioInfoList<SeUserInfo>* list = new AudioInfoList<SeUserInfo>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<SeUserInfo>;
    list->mInfos->allocBuffer(entryNum + 1, nullptr);
    mUserInfoList = list;
    for (s32 i = 0; i < entryNum; i++) {
        StringTmp<128> fileName;
        pResource->getEntryName(&fileName, "/", i);
        ByamlIter iter(static_cast<const u8*>(pResource->getOtherFile(fileName, nullptr)));
        fileName.removeSuffix(".byml");
        SeUserInfo* info = SeUserInfo::createInfo(iter, fileName);
        mUserInfoList->mInfos->pushBack(info);
    }

    mUserInfoList->sortInfo();
}

/**
 * Creates the SE resource specific information.
 * @param pResource SE database resource.
 * @param pPlayer Audio player used to read sound information.
 */
void SeDataBase::createResourceSespecificInfoList(const Resource* pResource, SeadAudioPlayer* pPlayer) {
    ByamlIter rootIter(pResource->getByml("SePropertyList"));
    s32 size = rootIter.getSize();
    AudioInfoList<SeResourceSpecificInfo>* list = new AudioInfoList<SeResourceSpecificInfo>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<SeResourceSpecificInfo>;
    list->mInfos->allocBuffer(size + 1, nullptr);
    mResourceSpecificInfoList = list;
    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;
        rootIter.tryGetIterByIndex(&iter, i);
        SeResourceSpecificInfo* info = SeResourceSpecificInfo::createInfo(iter);
        if (info == nullptr) {
            continue;
        }

        SoundInfo soundInfo;
        bool isRead = pPlayer->readSoundInfo(&soundInfo, info->mSoundId);
        info->mPlayerId = isRead ? static_cast<u8>(soundInfo.playerId) : 0;
        mResourceSpecificInfoList->mInfos->pushBack(info);
    }

    mResourceSpecificInfoList->sortInfo();
}

/**
 * Creates the loading information of the stationed archives.
 * @param pResource SE database resource.
 */
void SeDataBase::createStationedArchiveInfoList(const Resource* pResource) {
    ByamlIter iter(pResource->getByml("SeArchiveLoadingInfoList"));
    mArchiveLoadingInfoList = createInfoList<SeArchiveLoadingInfo>(iter);
}

/**
 * Links every SE resource information to its resource specific information.
 */
void SeDataBase::setResourceSpecInfoToResourceInfo() {
    for (s32 i = 0; i < (mUserInfoList != nullptr ? mUserInfoList->getInfoNum() : 0); i++) {
        const AudioInfoList<SePlayInfo>* playInfoList = mUserInfoList->getInfo(i)->mPlayInfoList;
        for (s32 j = 0; j < (playInfoList != nullptr ? playInfoList->getInfoNum() : 0); j++) {
            const AudioInfoList<SeResourceInfo>* resourceInfoList = playInfoList->getInfo(j)->mResourceInfoList;
            for (s32 k = 0; k < (resourceInfoList != nullptr ? resourceInfoList->getInfoNum() : 0); k++) {
                SeResourceInfo* resourceInfo = resourceInfoList->getInfo(k);
                resourceInfo->mSpecificInfo = tryFindResourceSpecificInfo(resourceInfo->mName);
            }
        }
    }
}

/**
 * Does nothing.
 * @param pLoader Resource loader.
 */
void SeDataBase::callLoadResourceStationedSound(IAudioResourceLoader* pLoader) const {}

/**
 * Does nothing.
 * @param pLoader Resource loader.
 * @param rName Bank list name.
 */
void SeDataBase::callLoadBankListBank(IAudioResourceLoader* pLoader, const sead::SafeString& rName) const {}

/**
 * Does nothing.
 */
void SeDataBase::requestLoadResourceStationedSound() const {}

/**
 * Does nothing.
 */
void SeDataBase::requestLoadResourceStationedObjectSound() const {}

/**
 * Does nothing.
 * @param rName Bank list name.
 */
void SeDataBase::requestLoadBankListBank(const sead::SafeString& rName) const {}

/**
 * Creates SE bank loading information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if the bank does not exist.
 */
SeBankLoadingInfo* SeBankLoadingInfo::createInfo(const ByamlIter& rIter) {
    SeBankLoadingInfo* info = new SeBankLoadingInfo;
    info->mName = nullptr;
    info->mSoundId = AudioConst::SOUND_ID_INVALID;
    rIter.tryGetStringByKey(&info->mName, "Name");
    bool isExist = alSoundNameUtil::isExistItemName(info->mName, true);
    info->mSoundId = alSoundNameUtil::getSoundId(info->mName, isExist);
    return AudioConst::SOUND_ID_INVALID == info->mSoundId ? nullptr : info;
}

/**
 * Creates SE user loading information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SeUserLoadingInfo* SeUserLoadingInfo::createInfo(const ByamlIter& rIter) {
    SeUserLoadingInfo* info = new SeUserLoadingInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    return info;
}

/**
 * Creates SE archive loading information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SeArchiveLoadingInfo* SeArchiveLoadingInfo::createInfo(const ByamlIter& rIter) {
    SeArchiveLoadingInfo* info = new SeArchiveLoadingInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter bankIter;
    info->mBankLoadingInfoList =
        rIter.tryGetIterByKey(&bankIter, "BankLoadingInfoList") ? createInfoList<SeBankLoadingInfo>(bankIter) : nullptr;
    ByamlIter userIter;
    if (rIter.tryGetIterByKey(&userIter, "UserLoadingInfoList")) {
        info->mUserLoadingInfoList = createInfoList<SeUserLoadingInfo>(userIter);
    } else {
        info->mUserLoadingInfoList = nullptr;
    }

    return info;
}

/**
 * Compares two SE bank loading information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeBankLoadingInfo::compareInfo(const SeBankLoadingInfo* pA, const SeBankLoadingInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two SE user loading information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeUserLoadingInfo::compareInfo(const SeUserLoadingInfo* pA, const SeUserLoadingInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two SE archive loading information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeArchiveLoadingInfo::compareInfo(const SeArchiveLoadingInfo* pA, const SeArchiveLoadingInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

}  // namespace al
