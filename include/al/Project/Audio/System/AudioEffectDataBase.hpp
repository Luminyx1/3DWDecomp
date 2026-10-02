#pragma once

#include <basis/seadTypes.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class SeEffectBusSettingInfo;
class SeEffectInfo;
class SeStageEffectInfo;

class AudioEffectDataBase {
public:
    AudioEffectDataBase(const char* pArchiveName, const char* pUnused);

    static const char* ARC_NAME;

    AudioInfoList<SeEffectBusSettingInfo>* mEffectBusSettingInfoList = nullptr;
    AudioInfoList<SeEffectInfo>* mEffectInfoList = nullptr;
    AudioInfoList<SeStageEffectInfo>* mStageEffectInfoList = nullptr;
};

static_assert(sizeof(AudioEffectDataBase) == 0x18);
}  // namespace al
