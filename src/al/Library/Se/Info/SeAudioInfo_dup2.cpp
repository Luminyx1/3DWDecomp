#include "Library/Se/Info/SeAudioInfo.hpp"

#include <attributes.h>
#include <cstdio>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Math/InOutParam.hpp"
#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Function/SeFunctionInternal.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
using ActionPlayInfoList = AudioInfoList<SePlayInfoInAction>;
namespace {
/**
 * @brief Creates an optional input-to-output parameter mapping from a BYAML child.
 * @param rIter Dictionary containing the optional parameter mapping.
 * @param pKey Child key identifying the pitch, volume, or tempo mapping.
 * @return Newly allocated mapping, or nullptr when the child is absent.
 */
inline InOutParam* createMappedParam(const ByamlIter& rIter, const char* pKey) {
    ByamlIter paramIter;
    InOutParam* pParam;
    if (rIter.tryGetIterByKey(&paramIter, pKey)) {
        pParam = new InOutParam(0.0f, 0.0f, 1.0f, 1.0f);
        pParam->init(paramIter);
    } else {
        pParam = nullptr;
    }
    return pParam;
}
} // namespace
} // namespace al

namespace al {
/**
 * @brief Creates SE user information from BYAML data.
 * @param rIter BYAML data.
 * @param rName User name.
 * @return Created information.
 */
SeUserInfo* SeUserInfo::createInfo(const ByamlIter& rIter, const sead::SafeString& rName) {
    SeUserInfo* pInfo = new SeUserInfo;
    s32 size = rName.calcLength() + 1;
    char* pName = new char[size];
    snprintf(pName, size, "%s", rName.cstr());
    pInfo->mName = pName;

    {
        ByamlIter actionIter;

        if (rIter.tryGetIterByKey(&actionIter, "ActionInfoList")) {
            pInfo->mActionInfoList = createInfoList<SeActionInfo>(actionIter);
        } else {
            pInfo->mActionInfoList = nullptr;
        }
    }

    {
        ByamlIter playIter;

        if (rIter.tryGetIterByKey(&playIter, "PlayInfoList")) {
            pInfo->mPlayInfoList = createInfoList<SePlayInfo>(playIter);
        } else {
            pInfo->mPlayInfoList = nullptr;
        }
    }

    {
        ByamlIter emitterIter;

        if (rIter.tryGetIterByKey(&emitterIter, "EmitterInfoList")) {
            pInfo->mEmitterInfoList = createInfoList<SeEmitterInfo>(emitterIter);
        } else {
            pInfo->mEmitterInfoList = alSeDbFunction::createDefaultEmitterInfoList();
        }
    }

    return pInfo;
}

/**
 * @brief Compares two SE user information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeUserInfo::compareInfo(const SeUserInfo* pA, const SeUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

} // namespace al

namespace al {
/**
 * @brief Plays an SE by name without the SE database.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pEmitterName Optional emitter name passed to the SE keeper.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                            const char* pEmitterName) {
    return detail::startSeOld(pUser, rName, pEmitterName);
}

/**
 * @brief Checks whether SE are forced to be invalid for an audio user.
 * @param pUser Audio user.
 * @return True if SE are invalid.
 */
bool isForceInvalidSe(const IUseAudioKeeper* pUser) { return detail::isForceInvalidSe(pUser); }

/**
 * @brief Holds an SE by name without the SE database.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pEmitterName Optional emitter name passed to the SE keeper.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                           const char* pEmitterName) {
    return detail::holdSeOld(pUser, rName, pEmitterName);
}

/**
 * @brief Plays an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 startSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * @brief Plays an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryStartSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * @brief Holds an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 holdSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * @brief Holds an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryHoldSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * @brief Plays an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                        f32 param, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, false, false);
}

/**
 * @brief Tries to play an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryStartSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                           f32 param, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, false, true);
}

/**
 * @brief Holds an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                                       MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, true, false);
}

/**
 * @brief Tries to hold an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryHoldSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                          f32 param, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, true, true);
}

/**
 * @brief Stops an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void stopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (!detail::isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * @brief Stops an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void tryStopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * @brief Plays an SE.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 startSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * @brief Plays an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryStartSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * @brief Holds an SE.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 holdSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * @brief Holds an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryHoldSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * @brief Plays an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                                  MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, false, false);
}

/**
 * @brief Tries to play an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryStartSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                                     MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, false, true);
}

/**
 * @brief Holds an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                                 MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, true, false);
}

/**
 * @brief Tries to hold an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryHoldSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                                    MeInfo* pMeInfo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param,
                                                                                  pMeInfo, true, true);
}

/**
 * @brief Stops an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void tryStopSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * @brief Plays an SE by play name and sets its pitch.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                       f32 pitch) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (pitch >= 0.0f) {
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * @brief Plays an SE by play name and sets its pitch, volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                                  f32 pitch, f32 volume, f32 tempo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList != nullptr) {
        if (volume >= 0.0f) {
            paramList->setVolume(volume);
        }

        if (pitch >= 0.0f) {
            paramList->setPitch(pitch);
        }

        if (tempo >= 0.0f) {
            paramList->setTempo(tempo);
        }
    }

    return paramList;
}

/**
 * @brief Plays an SE by play name and sets its volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                        f32 volume) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (volume >= 0.0f) {
        paramList->setVolume(volume);
    }

    return paramList;
}

/**
 * @brief Plays an SE by play name and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                             f32 pitch, f32 volume) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList != nullptr) {
        if (volume >= 0.0f) {
            paramList->setVolume(volume);
        }

        if (pitch >= 0.0f) {
            paramList->setPitch(pitch);
        }
    }

    return paramList;
}

/**
 * @brief Plays an SE by play name and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser,
                                                   const sead::SafeString& rName, s32 value, s16 index) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * @brief Plays an SE by play name and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                             f32 volume, f32 tempo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, false);
    if (paramList != nullptr) {
        if (volume >= 0.0f) {
            paramList->setVolume(volume);
        }

        if (tempo >= 0.0f) {
            paramList->setTempo(tempo);
        }
    }

    return paramList;
}

/**
 * @brief Holds an SE by play name and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch.
 * @param volume Volume.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                            f32 pitch, f32 volume) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * @brief Holds an SE by play name and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume.
 * @param tempo Tempo.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                            f32 volume, f32 tempo) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setTempo(tempo);
    }

    return paramList;
}

/**
 * @brief Holds an SE by play name and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                                  s32 value, s16 index) {
    if (!detail::isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList = pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(
        rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * @brief Sets the default value of a sequence local variable.
 * @param pUser Audio user.
 * @param index Variable index.
 * @param value Variable value.
 */
void setSeSeqLocalVariableDefault(const IUseAudioKeeper* pUser, s32 index, s32 value) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setSeqLocalVariableDefault(index, value);
}

/**
 * @brief Sets the default biquad filter.
 * @param pUser Audio user.
 * @param type Filter type.
 * @param value Filter value.
 */
void setSeBiquadFilterDefault(const IUseAudioKeeper* pUser, s32 type, f32 value) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setBiquadFilterDefault(type, value);
}

} // namespace al

namespace al {
/**
 * @brief Creates SE resource information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if the sound does not exist.
 */
SeResourceInfo* SeResourceInfo::createInfo(const ByamlIter& rIter) {
    SeResourceInfo* pInfo = new SeResourceInfo;
    rIter.tryGetStringByKey(&pInfo->mName, "Name");
    u32 soundId = alSoundNameUtil::getSoundId(pInfo->mName, false);
    pInfo->mSoundId = soundId;

    if (AudioConst::SOUND_ID_INVALID == soundId) {
        return nullptr;
    }

    const char* pInputFunctionName = nullptr;

    if (rIter.tryGetStringByKey(&pInputFunctionName, "InputFunctionName")) {
        pInfo->mInputFunctionId = alSeDbFunction::convertInputFunctionNameToId(pInputFunctionName);
    } else {
        pInfo->mInputFunctionId = 0;
    }

    pInfo->mPitch = createMappedParam(rIter, "Pitch");

    pInfo->mVolume = createMappedParam(rIter, "Volume");

    pInfo->mTempo = createMappedParam(rIter, "Tempo");

    if (!rIter.tryGetStringByKey(&pInfo->mEmitterName, "EmitterName")) {
        pInfo->mEmitterName = nullptr;
    }

    if (rIter.tryGetFloatByKey(&pInfo->mParamMin, "ParamMin")) {
        pInfo->mIsSetParamMin = true;
    }

    if (!rIter.tryGetIntByKey(&pInfo->mLocalVarNo, "LocalVarNo")) {
        pInfo->mLocalVarNo = -1;
    }

    if (!rIter.tryGetFloatByKey(&pInfo->mLfeSend, "LfeSend")) {
        pInfo->mLfeSend = 0.0f;
    }

    return pInfo;
}

/**
 * @brief Creates SE play information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if there is no resource information.
 */
NOINLINE SePlayInfo* SePlayInfo::createInfo(const ByamlIter& rIter) {
    SePlayInfo* pInfo = new SePlayInfo;
    rIter.tryGetStringByKey(&pInfo->mName, "Name");

    if (!rIter.tryGetBoolByKey(&pInfo->mIsLoop, "IsLevel") &&
        !rIter.tryGetBoolByKey(&pInfo->mIsLoop, "IsLoop")) {
        pInfo->mIsLoop = false;
    }

    if (!rIter.tryGetIntByKey(&pInfo->mFadeOutFrameNum, "FadeOutFrameNum")) {
        pInfo->mFadeOutFrameNum = USE_DEFAULT_FADE_OUT_FRAME_NUM;
    }

    if (!rIter.tryGetStringByKey(&pInfo->mRequestKeeperName, "RequestKeeperName")) {
        pInfo->mRequestKeeperName = nullptr;
    }

    ByamlIter resourceIter;

    if (!rIter.tryGetIterByKey(&resourceIter, "ResourceInfoList")) {
        return nullptr;
    }

    pInfo->mResourceInfoList = createInfoList<SeResourceInfo>(resourceIter);
    return pInfo;
}

/**
 * @brief Compares two SE resource information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeResourceInfo::compareInfo(const SeResourceInfo* pA, const SeResourceInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * @brief Creates SE play information of an action from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SePlayInfoInAction* SePlayInfoInAction::createInfo(const ByamlIter& rIter) {
    SePlayInfoInAction* pInfo = new SePlayInfoInAction;
    rIter.tryGetStringByKey(&pInfo->mName, "Name");

    if (!rIter.tryGetFloatByKey(&pInfo->mStartFrame, "StartFrame")) {
        pInfo->mStartFrame = 0.0f;
    }

    if (!rIter.tryGetFloatByKey(&pInfo->mEndFrame, "EndFrame")) {
        pInfo->mEndFrame = 0.0f;
    }

    if (!rIter.tryGetBoolByKey(&pInfo->mIsOneTime, "IsOneTime")) {
        pInfo->mIsOneTime = false;
    }

    return pInfo;
}

/**
 * @brief Compares two SE play information of an action by start frame.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SePlayInfoInAction::compareInfo(const SePlayInfoInAction* pA, const SePlayInfoInAction* pB) {
    if (pA->mStartFrame < pB->mStartFrame) {
        return -1;
    }

    return pA->mStartFrame > pB->mStartFrame;
}

/**
 * @brief Constructs empty SE play information of an action.
 */
SePlayInfoInAction::SePlayInfoInAction() = default;

/**
 * @brief Copies SE play information of an action.
 * @param rOther Information to copy.
 */
SePlayInfoInAction::SePlayInfoInAction(const SePlayInfoInAction& rOther)
    : mName(rOther.mName), mStartFrame(rOther.mStartFrame), mEndFrame(rOther.mEndFrame),
      mIsOneTime(rOther.mIsOneTime) {}

/**
 * @brief Copies SE play information of an action.
 * @param rOther Information to copy.
 * @return This information.
 */
SePlayInfoInAction& SePlayInfoInAction::operator=(const SePlayInfoInAction& rOther) {
    mName = rOther.mName;
    mStartFrame = rOther.mStartFrame;
    mEndFrame = rOther.mEndFrame;
    mIsOneTime = rOther.mIsOneTime;
    return *this;
}

/**
 * @brief Constructs empty SE action information.
 */
SeActionInfo::SeActionInfo() : mName(nullptr), mIsStopPlayingSe(false), mPlayInfoList(nullptr) {}

/**
 * @brief Copies SE action information including its play information.
 * @param rOther Information to copy.
 */
SeActionInfo::SeActionInfo(const SeActionInfo& rOther)
    : mName(rOther.mName), mIsStopPlayingSe(rOther.mIsStopPlayingSe) {
    if (rOther.mPlayInfoList == nullptr) {
        mPlayInfoList = nullptr;
        return;
    }

    s32 num = rOther.mPlayInfoList->getInfoNum();
    ActionPlayInfoList* pList = new ActionPlayInfoList;
    pList->mNext = nullptr;
    pList->mInfos = new sead::PtrArray<SePlayInfoInAction>;
    pList->mInfos->allocBuffer(num + 1, nullptr);
    mPlayInfoList = pList;

    if (num > 0) {
        for (s32 i = 0; i < num; i++) {
            SePlayInfoInAction* pInfo =
                rOther.mPlayInfoList != nullptr ? rOther.mPlayInfoList->getInfo(i) : nullptr;

            if (pInfo == nullptr) {
                continue;
            }

            SePlayInfoInAction* pCopy = new SePlayInfoInAction(*pInfo);
            mPlayInfoList->mInfos->pushBack(pCopy);
        }
    }
}

/**
 * @brief Copies SE action information.
 * @param rOther Information to copy.
 * @return This information.
 */
SeActionInfo& SeActionInfo::operator=(const SeActionInfo& rOther) {
    mName = rOther.mName;
    mIsStopPlayingSe = rOther.mIsStopPlayingSe;

    if (rOther.mPlayInfoList != nullptr && mPlayInfoList != nullptr) {
        *mPlayInfoList = *rOther.mPlayInfoList;
    }

    return *this;
}

/**
 * @brief Creates a copy of SE emitter information with copied names.
 * @param pInfo Non-null emitter information; offset copying is unsupported and traps if an offset exists.
 * @return Created information.
 */
SeEmitterInfo* SeEmitterInfo::duplicateInfo(const SeEmitterInfo* pInfo) {
    SeEmitterInfo* pCopy = new SeEmitterInfo;
    pCopy->mName = alSeDbFunction::createNameAreaAndCopy(pInfo->mName);
    pCopy->mJointName = alSeDbFunction::createNameAreaAndCopy(pInfo->mJointName);

    if (pInfo->mOffset != nullptr) {
        __builtin_trap();
    }

    const SeSoundSourceInfo* pSourceInfo = pInfo->mSoundSourceInfo;
    SeSoundSourceInfo* pSourceCopy = nullptr;

    if (pSourceInfo != nullptr && pSourceInfo->mName != nullptr) {
        const char* pName = alSeDbFunction::createNameAreaAndCopy(pSourceInfo->mName);

        if (pName != nullptr) {
            if (alSeFunction::isSoundSourceAmbient(pName)) {
                pSourceCopy = new SeSoundSourceInfoAmbient(pName);
            } else if (alSeFunction::isSoundSource3DPoint(pName)) {
                pSourceCopy = new SeSoundSourceInfo3DPoint(pName);
            } else if (alSeFunction::isSoundSource3DSphere(pName)) {
                pSourceCopy = new SeSoundSourceInfo3DSphere(
                    *static_cast<const SeSoundSourceInfo3DSphere*>(pSourceInfo));
            } else if (alSeFunction::isSoundSource3DVector(pName)) {
                pSourceCopy = new SeSoundSourceInfo3DVector(
                    *static_cast<const SeSoundSourceInfo3DVector*>(pSourceInfo));
            } else if (alSeFunction::isSoundSource3DBox(pName)) {
                pSourceCopy =
                    new SeSoundSourceInfo3DBox(*static_cast<const SeSoundSourceInfo3DBox*>(pSourceInfo));
            } else if (alSeFunction::isSoundSource3DRing(pName)) {
                pSourceCopy =
                    new SeSoundSourceInfo3DRing(*static_cast<const SeSoundSourceInfo3DRing*>(pSourceInfo));
            }
        }
    }

    pCopy->mSoundSourceInfo = pSourceCopy;
    return pCopy;
}

/**
 * @brief Does nothing.
 */
void SeSoundSourceInfo::dummy() {}
} // namespace al

namespace al {
template AudioInfoList<SePlayInfoInAction>* createInfoList<SePlayInfoInAction>(const ByamlIter&);
}
