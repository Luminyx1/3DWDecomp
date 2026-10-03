#include "Library/Se/Function/SeFunction.hpp"

#include "Library/Audio/AudioDirector.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Se/DataBase/SeDataBase.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Audio/System/AudioResourceLoader.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Se/SeEmitter.hpp"
#include "Project/Se/SeEmitterHolder.hpp"

namespace alSeFunction {
/**
 * @brief Checks whether a sound source type name is the ambient type.
 * @param pName Sound source type name.
 * @return True if the name is the ambient type.
 */
bool isSoundSourceAmbient(const char* pName) { return al::isEqualString(pName, "環境音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D point type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D point type.
 */
bool isSoundSource3DPoint(const char* pName) { return al::isEqualString(pName, "３Ｄ点音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D sphere type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D sphere type.
 */
bool isSoundSource3DSphere(const char* pName) { return al::isEqualString(pName, "３Ｄ球音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D line type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D line type.
 */
bool isSoundSource3DVector(const char* pName) { return al::isEqualString(pName, "３Ｄ線音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D plane rectangle type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D plane rectangle type.
 */
bool isSoundSource3DBox(const char* pName) { return al::isEqualString(pName, "３Ｄ平面長方形音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D ring type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D ring type.
 */
bool isSoundSource3DRing(const char* pName) { return al::isEqualString(pName, "３Ｄリング音源"); }

/**
 * @brief Checks whether a sound source type name is the 3D plane circle type.
 * @param pName Sound source type name.
 * @return True if the name is the 3D plane circle type.
 */
bool isSoundSource3DCircle(const char* pName) { return al::isEqualString(pName, "３Ｄ平面円音源"); }

/**
 * @brief Stops all SE except the given request keeper.
 * @param pDirector Audio director.
 * @param fadeFrames Fade out frames.
 * @param pExceptName Name of the request keeper that keeps playing.
 */
void stopAllSe(al::AudioDirector* pDirector, u32 fadeFrames, const char* pExceptName) {
    pDirector->getSeDirector()->stopAll(fadeFrames, nullptr, pExceptName);
}

/**
 * @brief Stops all SE of a request keeper.
 * @param pDirector Audio director.
 * @param pKeeperName Request keeper name.
 * @param fadeFrames Fade out frames.
 */
void stopAllSeWithExceptList(al::AudioDirector* pDirector, const char* pKeeperName, u32 fadeFrames) {
    pDirector->getSeDirector()->stopAll(fadeFrames, pKeeperName, nullptr);
}

/**
 * @brief Stops all SE except the given request keepers.
 * @param pDirector Audio director.
 * @param fadeFrames Fade out frames.
 * @param pExceptList Names of the request keepers that keep playing.
 * @param exceptNum Number of names.
 */
void stopAllSeExcept(al::AudioDirector* pDirector, u32 fadeFrames, const char** pExceptList, u32 exceptNum) {
    pDirector->getSeDirector()->stopAllExcept(fadeFrames, pExceptList, exceptNum);
}

/**
 * @brief Deactivates a request keeper.
 * @param pDirector Audio director.
 * @param pName Request keeper name.
 */
void deactivateRequestKeeper(al::AudioDirector* pDirector, const char* pName) {
    pDirector->getSeDirector()->deactivateRequestKeeper(pName);
}

/**
 * @brief Activates a request keeper.
 * @param pDirector Audio director.
 * @param pName Request keeper name.
 */
void activateRequestKeeper(al::AudioDirector* pDirector, const char* pName) {
    pDirector->getSeDirector()->activateRequestKeeper(pName);
}

/**
 * @brief Sets whether the stage is in the state after the goal.
 * @param pDirector Audio director.
 * @param isAfterGoal Whether the stage is in the state after the goal.
 */
void setIsStateAfterGoal(const al::AudioDirector* pDirector, bool isAfterGoal) {
    pDirector->getSeDirector()->setIsStateAfterGoal(isAfterGoal);
}

/**
 * @brief Sets whether the stage is in the state after the goal.
 * @param pUser Audio user.
 * @param isAfterGoal Whether the stage is in the state after the goal.
 */
void setIsStateAfterGoal(al::IUseAudioKeeper* pUser, bool isAfterGoal) {
    pUser->getAudioKeeper()->getSeKeeper()->getSeDirector()->setIsStateAfterGoal(isAfterGoal);
}

/**
 * @brief Excludes the SE that are not allowed in commercials.
 * @param pDirector Audio director.
 */
void setIsExcludeCmNgSe(const al::AudioDirector* pDirector) {
    pDirector->getSeDirector()->setIsExcludeCmNgSe();
}

/**
 * @brief Sets the volume setting of a request keeper.
 * @param pDirector Audio director.
 * @param pKeeperName Request keeper name.
 * @param pSettingName Volume setting name.
 * @param fadeFrames Fade frames.
 * @param isForce Whether to force the setting.
 */
void setRequestKeeperVolumeSetting(const al::AudioDirector* pDirector, const char* pKeeperName,
                                   const char* pSettingName, s32 fadeFrames, bool isForce) {
    pDirector->getSeDirector()->setVolumeSetting(pKeeperName, pSettingName, fadeFrames, isForce);
}

/**
 * @brief Sets the volume setting of a request keeper.
 * @param pUser Audio user.
 * @param pKeeperName Request keeper name.
 * @param pSettingName Volume setting name.
 * @param fadeFrames Fade frames.
 * @param isForce Whether to force the setting.
 */
void setRequestKeeperVolumeSetting(al::IUseAudioKeeper* pUser, const char* pKeeperName,
                                   const char* pSettingName, s32 fadeFrames, bool isForce) {
    pUser->getAudioKeeper()->getSeKeeper()->getSeDirector()->setVolumeSetting(pKeeperName, pSettingName,
                                                                              fadeFrames, isForce);
}

/**
 * @brief Sets the volume setting of all request keepers.
 * @param pUser Audio user.
 * @param pSettingName Volume setting name.
 * @param fadeFrames Fade frames.
 */
void setAllRequestKeeperVolumeSetting(al::IUseAudioKeeper* pUser, const char* pSettingName, s32 fadeFrames) {
    pUser->getAudioKeeper();
    pUser->getAudioKeeper()->getSeKeeper()->getSeDirector()->setAllKeeperVolumeSetting(pSettingName,
                                                                                       fadeFrames);
}

/**
 * @brief Gets the SE keeper of an audio user.
 * @param pUser Audio user.
 * @return SE keeper, or nullptr if the user has no audio keeper.
 */
al::SeKeeper* getSeKeeper(al::IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper() == nullptr) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper();
}

/**
 * @brief Loads the banks and the SE resources of the users of an archive.
 * @param pLoader Resource loader.
 * @param pArchiveInfo Archive loading information.
 * @param pUserInfoList SE user information list.
 * @param isUnused Unused.
 */
void loadSoundArchive(al::IAudioResourceLoader* pLoader, const al::SeArchiveLoadingInfo* pArchiveInfo,
                      const al::AudioInfoList<al::SeUserInfo>* pUserInfoList, bool isUnused) {
    const al::AudioInfoList<al::SeBankLoadingInfo>* bankList = pArchiveInfo->mBankLoadingInfoList;

    if (bankList != nullptr) {
        for (s32 i = 0; i < bankList->getInfoNum(); i++) {
            pLoader->loadSoundItem(bankList->getInfo(i)->mSoundId, -1);
        }
    }

    const al::AudioInfoList<al::SeUserLoadingInfo>* userList = pArchiveInfo->mUserLoadingInfoList;

    for (s32 i = 0; i < (userList != nullptr ? userList->getInfoNum() : 0); i++) {
        const char* name = userList->getInfo(i)->mName;
        const al::SeUserInfo* userInfo = nullptr;

        if (pUserInfoList != nullptr && name != nullptr) {
            userInfo = pUserInfoList->tryFindInfo(name);
        }

        const al::AudioInfoList<al::SePlayInfo>* playList =
            userInfo != nullptr ? userInfo->mPlayInfoList : nullptr;

        for (s32 j = 0; j < (playList != nullptr ? playList->getInfoNum() : 0); j++) {
            const al::AudioInfoList<al::SeResourceInfo>* resourceList =
                playList->getInfo(j)->mResourceInfoList;

            for (s32 k = 0; k < (resourceList != nullptr ? resourceList->getInfoNum() : 0); k++) {
                pLoader->loadSoundItem(resourceList->getInfo(k)->mSoundId, -1);
            }
        }
    }
}

/**
 * @brief Gets the SE source of an emitter.
 * @param pHolder Emitter holder.
 * @param index Emitter index.
 * @return SE source.
 */
al::SeSource* getSeSource(al::SeEmitterHolder* pHolder, s32 index) {
    return pHolder->getEmitter(index)->getSeSource();
}

/**
 * @brief Changes the listener poser to the demo one.
 * @param pDirector Audio director.
 */
void changeListenerPoserDemo(al::AudioDirector* pDirector) {
    pDirector->getSeDirector()->changeListenerPoser("カメラ位置オフセット");
}

/**
 * @brief Changes the listener poser back to the last one.
 * @param pDirector Audio director.
 */
void changeListenerPoserLast(al::AudioDirector* pDirector) {
    pDirector->getSeDirector()->changeListenerPoserToLast();
}

/**
 * @brief Deactivates the SE keeper of an audio user.
 * @param pUser Audio user.
 */
void deactivateSeKeeper(al::IUseAudioKeeper* pUser) {
    pUser->getAudioKeeper()->getSeKeeper()->deactivate(false);
}

/**
 * @brief Activates the SE keeper of an audio user.
 * @param pUser Audio user.
 */
void activateSeKeeper(al::IUseAudioKeeper* pUser) { pUser->getAudioKeeper()->getSeKeeper()->activate(); }
} // namespace alSeFunction

namespace al {
/**
 * @brief Creates the emitters of all emitter information.
 * @param pInfo Audio system information.
 * @param rName Unused.
 * @param pEmitterInfoList Emitter information list.
 * @param pModelKeeper Model keeper of the owner.
 * @param pPose SE source pose.
 * @param isUseModel Whether the emitters use the model.
 */
SeEmitterHolder::SeEmitterHolder(AudioSystemInfo* pInfo, const sead::SafeString& rName,
                                 const AudioInfoList<SeEmitterInfo>* pEmitterInfoList,
                                 const ModelKeeper* pModelKeeper, SeSourcePose* pPose, bool isUseModel) {
    mEmitters.allocBuffer(pEmitterInfoList != nullptr ? pEmitterInfoList->getInfoNum() : 0, nullptr);

    for (s32 i = 0; i < (pEmitterInfoList != nullptr ? pEmitterInfoList->getInfoNum() : 0); i++) {
        const SeEmitterInfo* emitterInfo = pEmitterInfoList->getInfo(i);
        mEmitters.pushBack(new SeEmitter(pInfo, emitterInfo, pModelKeeper, pPose, isUseModel));
    }
}

/**
 * @brief Updates all emitters.
 */
void SeEmitterHolder::update() {
    bool isAllEnd = true;

    for (s32 i = 0; i < mEmitters.size(); i++) {
        isAllEnd &= mEmitters.unsafeAt(i)->update();
    }

    if (isAllEnd) {
        mIsActive = false;
    }
}

} // namespace al
