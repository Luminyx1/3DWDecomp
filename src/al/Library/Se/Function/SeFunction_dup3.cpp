#include "Library/Se/Function/SeFunction.hpp"

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
bool isEnableSeKeeper(const IUseAudioKeeper* pUser) {
    return pUser->getAudioKeeper() != nullptr && pUser->getAudioKeeper()->getSeKeeper() != nullptr;
}
}  // namespace

/**
 * Plays an SE by name without the SE database.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pUnused Unused.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName, const char* pUnused) {
    if (isForceInvalidSe(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySe(
        alSoundNameUtil::getSoundId(rName.cstr(), false), pUnused, false, nullptr, nullptr, nullptr);
}

/**
 * Checks whether SE are forced to be invalid for an audio user.
 * @param pUser Audio user.
 * @return True if SE are invalid.
 */
bool isForceInvalidSe(const IUseAudioKeeper* pUser) {
    return pUser != nullptr && pUser->getAudioKeeper() != nullptr && pUser->getAudioKeeper()->isForceInvalidSe();
}

/**
 * Holds an SE by name without the SE database.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pUnused Unused.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName, const char* pUnused) {
    if (isForceInvalidSe(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestHoldSe(alSoundNameUtil::getSoundId(rName.cstr(), false),
                                                                 pUnused, nullptr, nullptr, nullptr);
}


/**
 * Plays an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 startSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * Plays an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryStartSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * Holds an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 holdSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * Holds an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryHoldSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * Plays an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  false, false);
}

/**
 * Tries to play an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryStartSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  false, true);
}

/**
 * Holds an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  true, false);
}

/**
 * Tries to hold an SE by play name with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryHoldSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  true, true);
}

/**
 * Stops an SE by play name.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void stopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (!isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * Stops an SE by play name without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void tryStopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (!isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * Plays an SE.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 startSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * Plays an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryStartSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, false);
}

/**
 * Holds an SE.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 holdSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * Holds an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pMeInfo Additional play information.
 * @return True if the request was made.
 */
s32 tryHoldSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromName(rName.cstr(), pMeInfo, true);
}

/**
 * Plays an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  false, false);
}

/**
 * Tries to play an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryStartSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  false, true);
}

/**
 * Holds an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  true, false);
}

/**
 * Tries to hold an SE with a parameter.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param param Play parameter.
 * @param pMeInfo Additional play information.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* tryHoldSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param, MeInfo* pMeInfo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameWithParam(rName.cstr(), param, pMeInfo,
                                                                                  true, true);
}

/**
 * Stops an SE without checking whether SE are invalid.
 * @param pUser Audio user.
 * @param rName Play name.
 */
void tryStopSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName) {
    if (!isEnableSeKeeper(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSeFromName(rName.cstr());
}

/**
 * Plays an SE by play name and sets its pitch.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (pitch >= 0.0f) {
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * Plays an SE by play name and sets its pitch, volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch, f32 volume, f32 tempo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
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
 * Plays an SE by play name and sets its volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (volume >= 0.0f) {
        paramList->setVolume(volume);
    }

    return paramList;
}

/**
 * Plays an SE by play name and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch, f32 volume) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
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
 * Plays an SE by play name and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value, s16 index) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * Plays an SE by play name and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume, f32 tempo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, false);
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
 * Holds an SE by play name and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param pitch Pitch.
 * @param volume Volume.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch, f32 volume) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * Holds an SE by play name and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param volume Volume.
 * @param tempo Tempo.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume, f32 tempo) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setTempo(tempo);
    }

    return paramList;
}

/**
 * Holds an SE by play name and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value, s16 index) {
    if (!isEnableSeKeeper(pUser)) {
        return nullptr;
    }

    SePlayParamList* paramList =
        pUser->getAudioKeeper()->getSeKeeper()->requestPlaySeFromNameGetParamList(rName.cstr(), nullptr, true);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * Sets the default value of a sequence local variable.
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
 * Sets the default biquad filter.
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

/**
 * Sets the volume of the SE source.
 * @param pUser Audio user.
 * @param volume Volume.
 */
void setSeSourceVolume(const IUseAudioKeeper* pUser, f32 volume) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setSeSourceVolume(volume);
}


/**
 * Plays an SE and sets its pitch.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitch(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (pitch >= 0.0f) {
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * Plays an SE and sets its pitch, volume and tempo.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch, f32 volume, f32 tempo) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
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
 * Plays an SE and sets its volume.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
    if (paramList == nullptr) {
        return nullptr;
    }

    if (volume >= 0.0f) {
        paramList->setVolume(volume);
    }

    return paramList;
}

/**
 * Plays an SE and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pitch Pitch, or a negative value to keep the default.
 * @param volume Volume, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetPitchVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                       f32 volume) {
    return startSeSetPitchVolumeTempo(pUser, rName, pitch, volume, -1.0f);
}

/**
 * Plays an SE and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetSeqLoacalVariable(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value, s16 index) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * Plays an SE and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param volume Volume, or a negative value to keep the default.
 * @param tempo Tempo, or a negative value to keep the default.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeSetVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume,
                                       f32 tempo) {
    return startSeSetPitchVolumeTempo(pUser, rName, -1.0f, volume, tempo);
}

/**
 * Holds an SE and sets its pitch and volume.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pitch Pitch.
 * @param volume Volume.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetPitchVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch, f32 volume) {
    SePlayParamList* paramList = holdSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * Holds an SE and sets its pitch.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param pitch Pitch.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetPitch(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch) {
    SePlayParamList* paramList = holdSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        paramList->setPitch(pitch);
    }

    return paramList;
}

/**
 * Holds an SE and sets its volume and tempo.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param volume Volume.
 * @param tempo Tempo.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume, f32 tempo) {
    SePlayParamList* paramList = holdSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        paramList->setVolume(volume);
        paramList->setTempo(tempo);
    }

    return paramList;
}

/**
 * Holds an SE and sets a sequence local variable.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param value Variable value.
 * @param index Variable index.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* holdSeSetSeqLoacalVariable(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value, s16 index) {
    SePlayParamList* paramList = holdSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        paramList->setLocalVariable(index, value);
    }

    return paramList;
}

/**
 * Stops an SE.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param fadeFrames Fade-out length in frames.
 */
void stopSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 fadeFrames) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopSe(alSoundNameUtil::getSoundId(rName.cstr(), false), fadeFrames,
                                                   nullptr, nullptr);
}

/**
 * Stops all SE of a play name.
 * @param pUser Audio user.
 * @param rName Play name.
 * @param fadeFrames Fade-out length in frames.
 * @param pUnused Unused.
 */
void stopAllSeId(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 fadeFrames, const char* pUnused) {
    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopAllSeFromName(rName.cstr(), fadeFrames, pUnused, false);
}

/**
 * Stops all SE of an audio user.
 * @param pUser Audio user.
 * @param fadeFrames Fade-out length in frames.
 */
void stopAllSeFromUser(const IUseAudioKeeper* pUser, s32 fadeFrames) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->stopAll(fadeFrames);
}

/**
 * Checks whether an audio user has an SE keeper.
 * @param pUser Audio user.
 * @return True if an SE keeper exists.
 */
bool isExistSeKeeper(const IUseAudioKeeper* pUser) {
    return pUser->getAudioKeeper() != nullptr && pUser->getAudioKeeper()->getSeKeeper() != nullptr;
}


/**
 * Updates the material code of the SE source.
 * @param pUser Audio user.
 * @param pMaterialName Material name.
 */
void tryUpdateSeMaterialCode(IUseAudioKeeper* pUser, const char* pMaterialName) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->tryUpdateMaterial(pMaterialName);
}

/**
 * Sets whether the SE source is in water.
 * @param pUser Audio user.
 * @param isInWater Whether the source is in water.
 */
void updateSeMaterialWater(IUseAudioKeeper* pUser, bool isInWater) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setIsInWater(isInWater);
}

/**
 * Sets whether the material of the SE source is wet.
 * @param pUser Audio user.
 * @param isWet Whether the material is wet.
 */
void updateSeMaterialWet(IUseAudioKeeper* pUser, bool isWet) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setIsMaterialWet(isWet);
}

/**
 * Sets whether the material of the SE source is wet in single mode.
 * @param pUser Audio user.
 * @param isWet Whether the material is wet.
 */
void updateSeMaterialWetSingleMode(IUseAudioKeeper* pUser, bool isWet) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setIsMaterialWetSingleMode(isWet);
}

/**
 * Sets whether the SE source is beyond a wall.
 * @param pUser Audio user.
 * @param isBeyondWall Whether the source is beyond a wall.
 */
void updateSeMaterialBeyondWall(IUseAudioKeeper* pUser, bool isBeyondWall) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setIsBeyondWall(isBeyondWall);
}

/**
 * Clears the material name of the SE source.
 * @param pUser Audio user.
 */
void resetSeMaterialName(const IUseAudioKeeper* pUser) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->resetMaterialName();
}

/**
 * Checks the material name of the SE source.
 * @param pUser Audio user.
 * @param pMaterialName Material name.
 * @return True if the names are equal.
 */
bool isCurSeMaterialNameEqual(const IUseAudioKeeper* pUser, const char* pMaterialName) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    const char* curName = pUser->getAudioKeeper()->getSeKeeper()->getMaterialName();
    return isEqualString(curName != nullptr ? curName : "NULL", pMaterialName);
}

/**
 * Checks whether the material of the SE source is wet in single mode.
 * @param pUser Audio user.
 * @return True if wet.
 */
bool isSeMaterialWetSingleModeSet(IUseAudioKeeper* pUser) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return false;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->isMaterialWetSingleMode();
}
}  // namespace al

namespace al {
bool isPadExistDeviceSpeaker(s32 port);

/**
 * Checks whether the user information of the SE keeper has an action.
 * @param pUser Audio user.
 * @param pActionName Action name.
 * @return True if the action exists.
 */
bool isExistSeActionNameInUserInfo(const IUseAudioKeeper* pUser, const char* pActionName) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    const SeUserInfo* userInfo = pUser->getAudioKeeper()->getSeKeeper()->getUserInfo();
    if (userInfo == nullptr) {
        return false;
    }

    const AudioInfoList<SeActionInfo>* actionInfoList = userInfo->mActionInfoList;
    if (actionInfoList == nullptr) {
        return false;
    }

    for (s32 i = 0; i < actionInfoList->getInfoNum(); i++) {
        const SeActionInfo* actionInfo = actionInfoList->getInfo(i);
        if (actionInfo != nullptr && isEqualString(actionInfo->mName, pActionName)) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether the user information of the SE keeper uses a resource.
 * @param pUser Audio user.
 * @param pResourceName Resource name.
 * @return True if the resource is used.
 */
bool isExistSeResourceNameInUserInfo(const IUseAudioKeeper* pUser, const char* pResourceName) {
    if (isForceInvalidSe(pUser)) {
        return false;
    }

    const SeUserInfo* userInfo = pUser->getAudioKeeper()->getSeKeeper()->getUserInfo();
    if (userInfo == nullptr) {
        return false;
    }

    const AudioInfoList<SePlayInfo>* playInfoList = userInfo->mPlayInfoList;
    if (playInfoList == nullptr) {
        return false;
    }

    for (s32 i = 0; i < playInfoList->getInfoNum(); i++) {
        const SePlayInfo* playInfo = playInfoList->getInfo(i);
        if (playInfo == nullptr) {
            continue;
        }

        const AudioInfoList<SeResourceInfo>* resourceInfoList = playInfo->mResourceInfoList;
        if (resourceInfoList == nullptr) {
            continue;
        }

        for (s32 j = 0; j < resourceInfoList->getInfoNum(); j++) {
            const SeResourceInfo* resourceInfo = resourceInfoList->getInfo(j);
            if (resourceInfo != nullptr && isEqualString(resourceInfo->mName, pResourceName)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Checks whether the user information of the SE keeper has a play name.
 * @param pUser Audio user.
 * @param pPlayName Play name.
 * @return True if the play name exists.
 */
bool isExistSePlayNameInUserInfo(const IUseAudioKeeper* pUser, const char* pPlayName) {
    if (pUser == nullptr) {
        return false;
    }

    if (isForceInvalidSe(pUser)) {
        return false;
    }

    if (pUser->getAudioKeeper() == nullptr) {
        return false;
    }

    if (pUser->getAudioKeeper()->getSeKeeper() == nullptr) {
        return false;
    }

    const SeUserInfo* userInfo = pUser->getAudioKeeper()->getSeKeeper()->getUserInfo();
    if (userInfo == nullptr) {
        return false;
    }

    const AudioInfoList<SePlayInfo>* playInfoList = userInfo->mPlayInfoList;
    if (playInfoList == nullptr) {
        return false;
    }

    for (s32 i = 0; i < playInfoList->getInfoNum(); i++) {
        const SePlayInfo* playInfo = playInfoList->getInfo(i);
        if (playInfo != nullptr && isEqualString(playInfo->mName, pPlayName)) {
            return true;
        }
    }

    return false;
}

/**
 * Sets the SE modifier of the SE keeper.
 * @param pUser Audio user.
 * @param pModifier SE modifier.
 */
void setSeModifier(const IUseAudioKeeper* pUser, ISeModifier* pModifier) {
    if (isForceInvalidSe(pUser)) {
        return;
    }

    pUser->getAudioKeeper()->getSeKeeper()->setModifier(pModifier);
}

/**
 * Does nothing in this version.
 * @param pParamList Parameter list.
 * @param port Controller port.
 * @param isRemote Whether to use the controller speaker.
 */
void setSeOutputFromController(SePlayParamList* pParamList, s32 port, bool isRemote) {}

/**
 * Does nothing in this version.
 * @param pParamList Parameter list.
 */
void setSeOutputTvDrcRemoteAll(SePlayParamList* pParamList) {}

/**
 * Plays an SE from a controller.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param port Controller port.
 * @param isRemote Whether to use the controller speaker.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeFromController(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 port,
                                       bool isRemote) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
    setSeOutputFromController(paramList, port, isRemote);
    return paramList;
}

/**
 * Plays an SE from a controller if it has a speaker, otherwise from all outputs.
 * @param pUser Audio user.
 * @param rName SE name.
 * @param port Controller port.
 * @return Parameter list of the request, or nullptr.
 */
SePlayParamList* startSeFromControllerOrTvDrc(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                              s32 port) {
    SePlayParamList* paramList = startSeOld(pUser, rName, nullptr);
    if (paramList != nullptr) {
        if (isPadExistDeviceSpeaker(port)) {
            setSeOutputFromController(paramList, port, false);
        } else {
            setSeOutputTvDrcRemoteAll(paramList);
        }
    }

    return paramList;
}
}  // namespace al
