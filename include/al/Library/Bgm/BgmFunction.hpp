#pragma once

namespace al {
class BgmDataBase;
class IAudioResourceLoader;
class SeadAudioPlayer;
}  // namespace al

namespace alBgmFunction {
void printBgmLineInfoList(const al::BgmDataBase* pDataBase);
void printBgmPlayInfoList(const al::BgmDataBase* pDataBase);
void printBgmResourceInfoList(const al::BgmDataBase* pDataBase);
void printBgmStageInfoList(const al::BgmDataBase* pDataBase);
void printBgmSituationInfoList(const al::BgmDataBase* pDataBase);
void printBgmUserInfoList(const al::BgmDataBase* pDataBase);
bool isSequenceSound(const char* pName);
bool isWaveSound(const char* pName);
bool isStreamSound(const char* pName);
bool tryLoadIfWaveSound(const char* pName, al::IAudioResourceLoader* pLoader, al::SeadAudioPlayer* pPlayer);
bool checkLoadIfWaveSound(const char* pName, al::SeadAudioPlayer* pPlayer);
bool isPlayingBgmByUpperLayerAudioUser(const al::BgmDataBase* pDataBase, const char* pName);
}  // namespace alBgmFunction
