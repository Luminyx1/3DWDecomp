#pragma once

#include "audio/seadAudioSubsetBase.h"
#include "container/seadOffsetList.h"
#include "framework/seadTaskParameter.h"

namespace sead {
class AudioPlayer;
class AudioResetter;
class AudioResourceLoader;
class AudioSystem;

class AudioSettingParameter : public TaskParameter {
    SEAD_RTTI_OVERRIDE(AudioSettingParameter, TaskParameter)

public:
    AudioSettingParameter();
    virtual ~AudioSettingParameter() {}

    void setAudioSystem(AudioSystem* pSystem);
    void setResetter(AudioResetter* pResetter);
    void setPlayer(AudioPlayer* pPlayer);
    void setResourceLoader(AudioResourceLoader* pLoader);
    void appendSubset(AudioSubsetBase* pSubset);

private:
    friend class AudioMgr;

    AudioSystem* mAudioSystem = nullptr;
    AudioResetter* mResetter = nullptr;
    AudioPlayer* mPlayer = nullptr;
    AudioResourceLoader* mResourceLoader = nullptr;
    OffsetList<AudioSubsetBase> mSubsetList;
};
static_assert(sizeof(AudioSettingParameter) == 0x40);
}  // namespace sead
