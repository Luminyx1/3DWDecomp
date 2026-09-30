#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class ISeModifier;
class MeInfo;
class SePlayParamList;

SePlayParamList* startSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName, const char* pUnused);
bool isForceInvalidSe(const IUseAudioKeeper* pUser);
SePlayParamList* holdSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName, const char* pUnused);
s32 startSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 tryStartSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 holdSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 tryHoldSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
SePlayParamList* startSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                            MeInfo* pMeInfo = nullptr);
SePlayParamList* tryStartSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                               MeInfo* pMeInfo = nullptr);
SePlayParamList* holdSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                           MeInfo* pMeInfo = nullptr);
SePlayParamList* tryHoldSeByNameWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                              MeInfo* pMeInfo = nullptr);
void stopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName);
void tryStopSeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName);
s32 startSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 tryStartSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 holdSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
s32 tryHoldSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, MeInfo* pMeInfo = nullptr);
SePlayParamList* startSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                      MeInfo* pMeInfo = nullptr);
SePlayParamList* tryStartSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                         MeInfo* pMeInfo = nullptr);
SePlayParamList* holdSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                     MeInfo* pMeInfo = nullptr);
SePlayParamList* tryHoldSeWithParam(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 param,
                        MeInfo* pMeInfo = nullptr);
void tryStopSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName);
SePlayParamList* startSeSetPitchByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch);
SePlayParamList* startSeSetPitchVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                                  f32 pitch, f32 volume, f32 tempo);
SePlayParamList* startSeSetVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume);
SePlayParamList* startSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                             f32 volume);
SePlayParamList* startSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                                   s32 value, s16 index);
SePlayParamList* startSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume,
                                             f32 tempo);
SePlayParamList* holdSeSetPitchVolumeByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                            f32 volume);
SePlayParamList* holdSeSetVolumeTempoByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume,
                                            f32 tempo);
SePlayParamList* holdSeSetSeqLoacalVariableByName(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                                  s32 value, s16 index);
void setSeSeqLocalVariableDefault(const IUseAudioKeeper* pUser, s32 index, s32 value);
void setSeBiquadFilterDefault(const IUseAudioKeeper* pUser, s32 type, f32 value);
void setSeSourceVolume(const IUseAudioKeeper* pUser, f32 volume);
SePlayParamList* startSeSetPitch(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch);
SePlayParamList* startSeSetPitchVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                            f32 volume, f32 tempo);
SePlayParamList* startSeSetVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume);
SePlayParamList* startSeSetPitchVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                       f32 volume);
SePlayParamList* startSeSetSeqLoacalVariable(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value,
                                             s16 index);
SePlayParamList* startSeSetVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume,
                                       f32 tempo);
SePlayParamList* holdSeSetPitchVolume(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch,
                                      f32 volume);
SePlayParamList* holdSeSetPitch(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 pitch);
SePlayParamList* holdSeSetVolumeTempo(const IUseAudioKeeper* pUser, const sead::SafeString& rName, f32 volume,
                                      f32 tempo);
SePlayParamList* holdSeSetSeqLoacalVariable(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 value,
                                            s16 index);
void stopSe(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 fadeFrames);
void stopAllSeId(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 fadeFrames, const char* pUnused);
void stopAllSeFromUser(const IUseAudioKeeper* pUser, s32 fadeFrames);
bool isExistSeKeeper(const IUseAudioKeeper* pUser);
void tryUpdateSeMaterialCode(IUseAudioKeeper* pUser, const char* pMaterialName);
void updateSeMaterialWater(IUseAudioKeeper* pUser, bool isInWater);
void updateSeMaterialWet(IUseAudioKeeper* pUser, bool isWet);
void updateSeMaterialWetSingleMode(IUseAudioKeeper* pUser, bool isWet);
void updateSeMaterialBeyondWall(IUseAudioKeeper* pUser, bool isBeyondWall);
void resetSeMaterialName(const IUseAudioKeeper* pUser);
bool isCurSeMaterialNameEqual(const IUseAudioKeeper* pUser, const char* pMaterialName);
bool isSeMaterialWetSingleModeSet(IUseAudioKeeper* pUser);
bool isExistSeActionNameInUserInfo(const IUseAudioKeeper* pUser, const char* pActionName);
bool isExistSeResourceNameInUserInfo(const IUseAudioKeeper* pUser, const char* pResourceName);
bool isExistSePlayNameInUserInfo(const IUseAudioKeeper* pUser, const char* pPlayName);
void setSeModifier(const IUseAudioKeeper* pUser, ISeModifier* pModifier);
void setSeOutputFromController(SePlayParamList* pParamList, s32 port, bool isRemote);
void setSeOutputTvDrcRemoteAll(SePlayParamList* pParamList);
SePlayParamList* startSeFromController(const IUseAudioKeeper* pUser, const sead::SafeString& rName, s32 port,
                                       bool isRemote);
SePlayParamList* startSeFromControllerOrTvDrc(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                              s32 port);
}  // namespace al

namespace alSeFunction {
bool isSoundSourceAmbient(const char* pName);
bool isSoundSource3DPoint(const char* pName);
bool isSoundSource3DSphere(const char* pName);
bool isSoundSource3DVector(const char* pName);
bool isSoundSource3DBox(const char* pName);
bool isSoundSource3DRing(const char* pName);
bool isSoundSource3DCircle(const char* pName);
}  // namespace alSeFunction

namespace al {
class AudioDirector;
class IAudioResourceLoader;
class SeArchiveLoadingInfo;
class SeEmitterHolder;
class SeKeeper;
class SeSource;
class SeUserInfo;
template <typename T>
class AudioInfoList;
}  // namespace al

namespace alSeFunction {
void stopAllSe(al::AudioDirector* pDirector, u32 fadeFrames, const char* pKeeperName);
void stopAllSeWithExceptList(al::AudioDirector* pDirector, const char* pExceptName, u32 fadeFrames);
void stopAllSeExcept(al::AudioDirector* pDirector, u32 fadeFrames, const char** pExceptList, u32 exceptNum);
void deactivateRequestKeeper(al::AudioDirector* pDirector, const char* pName);
void activateRequestKeeper(al::AudioDirector* pDirector, const char* pName);
void setIsStateAfterGoal(const al::AudioDirector* pDirector, bool isAfterGoal);
void setIsStateAfterGoal(al::IUseAudioKeeper* pUser, bool isAfterGoal);
void setIsExcludeCmNgSe(const al::AudioDirector* pDirector);
void setRequestKeeperVolumeSetting(const al::AudioDirector* pDirector, const char* pKeeperName,
                                   const char* pSettingName, s32 fadeFrames, bool isForce);
void setRequestKeeperVolumeSetting(al::IUseAudioKeeper* pUser, const char* pKeeperName, const char* pSettingName,
                                   s32 fadeFrames, bool isForce);
void setAllRequestKeeperVolumeSetting(al::IUseAudioKeeper* pUser, const char* pSettingName, s32 fadeFrames);
al::SeKeeper* getSeKeeper(al::IUseAudioKeeper* pUser);
void loadSoundArchive(al::IAudioResourceLoader* pLoader, const al::SeArchiveLoadingInfo* pArchiveInfo,
                      const al::AudioInfoList<al::SeUserInfo>* pUserInfoList, bool isUnused);
al::SeSource* getSeSource(al::SeEmitterHolder* pHolder, s32 index);
void changeListenerPoserDemo(al::AudioDirector* pDirector);
void changeListenerPoserLast(al::AudioDirector* pDirector);
void deactivateSeKeeper(al::IUseAudioKeeper* pUser);
void activateSeKeeper(al::IUseAudioKeeper* pUser);
}  // namespace alSeFunction
