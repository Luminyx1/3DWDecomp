#include "Library/Bgm/BgmFunction.hpp"

#include <audio/seadAudioSoundDataMgrNin.h>

#include "Library/Bgm/BgmDataBase.hpp"
#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioResourceLoader.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"
#include "Project/Bgm/BgmInfo.hpp"

namespace alBgmFunction {
/**
 * Walks the BGM line information list.
 * @param pDataBase BGM database.
 */
void printBgmLineInfoList(const al::BgmDataBase* pDataBase) {
    const al::AudioInfoList<al::BgmCombinedLineInfo>* combinedList = pDataBase->mCombinedLineInfoList;
    for (u32 i = 0; i < static_cast<u32>(combinedList->getInfoNum()); i++) {
        const al::AudioInfoList<al::BgmLineInfo>* lineList = combinedList->getInfo(i)->mLineInfoList;
        for (u32 j = 0; j < static_cast<u32>(lineList->getInfoNum()); j++) {
            lineList->tryGetInfo(j);
        }
    }
}

/**
 * Walks the BGM play information list.
 * @param pDataBase BGM database.
 */
void printBgmPlayInfoList(const al::BgmDataBase* pDataBase) {
    const al::AudioInfoList<al::BgmPlayInfo>* playList = pDataBase->mPlayInfoList;
    for (u32 i = 0; i < static_cast<u32>(playList->getInfoNum()); i++) {
        playList->tryGetInfo(i);
    }
}

/**
 * Walks the BGM resource information list.
 * @param pDataBase BGM database.
 */
void printBgmResourceInfoList(const al::BgmDataBase* pDataBase) {
    const al::AudioInfoList<al::BgmResourceInfo>* resourceList = pDataBase->mResourceInfoList;
    for (u32 i = 0; i < static_cast<u32>(resourceList->getInfoNum()); i++) {
        const al::BgmResourceInfo* info = resourceList->tryGetInfo(i);
        const al::AudioInfoList<al::BgmResourceSuffixInfo>* suffixList = info->mResourceSuffixInfoList;
        if (suffixList != nullptr) {
            for (u32 j = 0; j < static_cast<u32>(suffixList->getInfoNum()); j++) {
                suffixList->tryGetInfo(j);
            }
        }

        const al::AudioInfoList<al::BgmEnableSituationInfo>* enableList = info->mEnableSituationInfoList;
        if (enableList != nullptr) {
            for (u32 j = 0; j < static_cast<u32>(enableList->getInfoNum()); j++) {
                enableList->tryGetInfo(j);
            }
        }

        const al::AudioInfoList<al::BgmStartTriggerSituationInfo>* triggerList = info->mStartTriggerSituationInfoList;
        if (triggerList != nullptr) {
            for (u32 j = 0; j < static_cast<u32>(triggerList->getInfoNum()); j++) {
                triggerList->tryGetInfo(j);
            }
        }
    }
}

/**
 * Walks the BGM stage information list.
 * @param pDataBase BGM database.
 */
void printBgmStageInfoList(const al::BgmDataBase* pDataBase) {
    const al::AudioInfoList<al::BgmStageInfo>* stageList = pDataBase->mStageInfoList;
    for (u32 i = 0; i < static_cast<u32>(stageList->getInfoNum()); i++) {
        const al::AudioInfoList<al::BgmStagePlayInfo>* playList = stageList->getInfo(i)->mStagePlayInfoList;
        for (u32 j = 0; j < static_cast<u32>(playList->getInfoNum()); j++) {
            playList->tryGetInfo(j);
        }
    }
}

/**
 * Does nothing.
 * @param pDataBase BGM database.
 */
void printBgmSituationInfoList(const al::BgmDataBase* pDataBase) {}

/**
 * Walks the BGM user information list.
 * @param pDataBase BGM database.
 */
void printBgmUserInfoList(const al::BgmDataBase* pDataBase) {
    const sead::PtrArray<al::BgmUserInfo>* userList = pDataBase->mUserInfoList;
    for (s32 i = 0; i < userList->size(); i++) {
        const sead::PtrArray<al::BgmActionInfo>* actionList = userList->unsafeAt(i)->mActionInfoList;
        for (s32 j = 0; j < actionList->size(); j++) {
            const al::AudioInfoList<al::BgmPlayInfoInAction>* playList = actionList->unsafeAt(j)->mPlayInfoList;
            if (playList == nullptr) {
                continue;
            }

            for (s32 k = 0; k < playList->getInfoNum(); k++) {
                playList->tryGetInfo(k);
            }
        }
    }
}

/**
 * Checks whether a BGM is a sequence sound.
 * @param pName Sound name.
 * @return True if the BGM is a sequence sound.
 */
bool isSequenceSound(const char* pName) {
    return alSoundNameUtil::getSoundType(alSoundNameUtil::getSoundId(pName, true), true) == 1;
}

/**
 * Checks whether a BGM is a wave sound.
 * @param pName Sound name.
 * @return True if the BGM is a wave sound.
 */
bool isWaveSound(const char* pName) {
    return alSoundNameUtil::getSoundType(alSoundNameUtil::getSoundId(pName, true), true) == 3;
}

/**
 * Checks whether a BGM is a stream sound.
 * @param pName Sound name.
 * @return True if the BGM is a stream sound.
 */
bool isStreamSound(const char* pName) {
    return alSoundNameUtil::getSoundType(alSoundNameUtil::getSoundId(pName, true), true) == 2;
}

/**
 * Loads a BGM if it is a wave sound that is not loaded yet.
 * @param pName Sound name.
 * @param pLoader Resource loader.
 * @param pPlayer Audio player.
 * @return True if the BGM is loaded.
 */
bool tryLoadIfWaveSound(const char* pName, al::IAudioResourceLoader* pLoader, al::SeadAudioPlayer* pPlayer) {
    if (!isWaveSound(pName)) {
        return true;
    }

    if (pPlayer->getSoundDataMgr()->IsDataLoaded(pName, -1)) {
        return true;
    }

    return pLoader->loadSoundItem(alSoundNameUtil::getSoundId(pName, true), -1);
}

/**
 * Checks whether a BGM is loaded if it is a wave sound.
 * @param pName Sound name.
 * @param pPlayer Audio player.
 * @return True if the BGM is not a wave sound or is loaded.
 */
bool checkLoadIfWaveSound(const char* pName, al::SeadAudioPlayer* pPlayer) {
    if (!isWaveSound(pName)) {
        return true;
    }

    return pPlayer->getSoundDataMgr()->IsDataLoaded(pName, -1);
}

/**
 * Checks whether a BGM is played by the upper layer audio user.
 * @param pDataBase BGM database.
 * @param pName BGM play name.
 * @return True if the BGM is played by the upper layer audio user.
 */
bool isPlayingBgmByUpperLayerAudioUser(const al::BgmDataBase* pDataBase, const char* pName) {
    const al::BgmPlayInfo* info = nullptr;
    if (pName != nullptr && pDataBase->mPlayInfoList != nullptr) {
        info = pDataBase->mPlayInfoList->tryFindInfo(pName);
    }

    if (info == nullptr) {
        return false;
    }

    return info->mIsPlayingByUpperLayerAudioUser;
}
}  // namespace alBgmFunction
