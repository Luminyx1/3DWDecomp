#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioDirector;
class AudioSystemInfo;
struct GameSystemInfo;
class IUseAudioKeeper;
class IUseSeadAudioPlayer;
class PadRumbleDirector;
class SeadAudioPlayer;
}  // namespace al

namespace alSeFunction {
enum DemoType : s32;
}

namespace alAudioSystemFunction {
al::AudioSystemInfo* getAudioSystemInfo(const al::GameSystemInfo* pInfo);
u32 loadResourceFromUserManagementFile(const char* pFileName, al::SeadAudioPlayer* pPlayer, bool isAttachMemoryPool);
bool tryAttachMemoryPool(const char* pFileName, al::SeadAudioPlayer* pPlayer);
bool pauseBySystemError(const al::AudioDirector* pDirector, const al::AudioDirector* pSubDirector, bool isPause,
                        u32 fadeFrames);
void pauseSystem(const al::AudioDirector* pDirector, const al::IUseAudioKeeper* pUser, bool isPause, u32 fadeFrames);
void pauseSystemForDebug(const al::AudioDirector* pDirector, const al::IUseAudioKeeper* pUser, bool isPause,
                         u32 fadeFrames);
void pauseAudioDirectorForDebug(al::AudioDirector* pDirector, bool isPause, u32 fadeFrames);
void startDemo(al::AudioDirector* pDirector, alSeFunction::DemoType type);
bool isInDemo(al::AudioDirector* pDirector);
void endDemo(al::AudioDirector* pDirector, alSeFunction::DemoType type);
void changeDemo(al::AudioDirector* pDirector, alSeFunction::DemoType from, alSeFunction::DemoType to);
void forceStopDemoSe(al::AudioDirector* pDirector);
void softReset(const al::AudioDirector* pDirector, const al::AudioDirector* pSubDirector);
void stopAllSeAfterDemoSkip(const al::AudioDirector* pDirector, u32 fadeFrames);
s32 getSeSoundHeapUsedSize(const al::AudioDirector* pDirector);
s32 getBgmSoundHeapUsedSize(const al::AudioDirector* pDirector);
u64 getHeapFreeSize(const al::AudioDirector* pDirector);
u64 getHeapSize(const al::AudioDirector* pDirector);
bool loadSoundItem(al::IUseSeadAudioPlayer* pUser, u32 id, u32 loadFlag);
bool isLoadedSoundItem(al::IUseSeadAudioPlayer* pUser, u32 id);
s32 saveHeapState(al::IUseSeadAudioPlayer* pUser);
void loadHeapState(al::IUseSeadAudioPlayer* pUser, s32 level);
s32 getCurrentHeapStateLevel(al::IUseSeadAudioPlayer* pUser);
u64 getSoundResourceHeapFreeSize(al::IUseSeadAudioPlayer* pUser);
al::SeadAudioPlayer* tryFindAudioPlayerRegistedSoundMemoryPoolHandler(const char* pFileName,
                                                                      al::SeadAudioPlayer* pPlayer,
                                                                      al::SeadAudioPlayer* pSubPlayer);
bool tryDisableSoundMemoryPoolHandlerByFilePath(const char* pPath, al::SeadAudioPlayer* pPlayer);
void setPadRumbleDirectorForSe(al::AudioDirector* pDirector, al::PadRumbleDirector* pRumbleDirector);
}  // namespace alAudioSystemFunction

namespace al {
class ActorInitInfo;
class AudioKeeper;

AudioKeeper* createAudioKeeper(const char* pName, const ActorInitInfo& rInfo);
AudioKeeper* createAudioKeeper(const char* pName, const AudioDirector* pDirector);
void changeAudioEffect(const IUseAudioKeeper* pUser, const char* pName);
void changeSequenceAudioEffectWithAreaCheck(const IUseAudioKeeper* pUser);
void changeAudioEffectWithAreaCheck(const IUseAudioKeeper* pUser);
const char* getCurAudioEffectName(const IUseAudioKeeper* pUser);
void activateAudioEventController(const IUseAudioKeeper* pUser);
void deactivateAudioEventController(const IUseAudioKeeper* pUser);
void activateSePlayEvent(const IUseAudioKeeper* pUser);
void deactivateSePlayEvent(const IUseAudioKeeper* pUser);
void activateAudioEffectChangeEvent(const IUseAudioKeeper* pUser);
void deactivateAudioEffectChangeEvent(const IUseAudioKeeper* pUser);
}  // namespace al
