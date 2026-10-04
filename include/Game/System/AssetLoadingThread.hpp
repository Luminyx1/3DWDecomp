#pragma once
#include <basis/seadTypes.h>
#include <thread/seadEvent.h>
namespace al {
class AudioSystemInfo;
} // namespace al
namespace rc {
class AssetLoadingThread {
  public:
    enum LOAD_TYPE : unsigned int;
    bool isLoading(LOAD_TYPE type);
    bool isLoadDone(LOAD_TYPE type);
    bool isLoadOrLoading(LOAD_TYPE type);
    void disableFastLoad(bool disable);

    /**
     * @brief Set the audio system info used while loading assets.
     * @param pInfo The audio system info.
     */
    void setAudioSystemInfo(al::AudioSystemInfo* pInfo) { mpAudioSystemInfo = pInfo; }

    static AssetLoadingThread* sInstance;

  private:
    u8 mUnreconstructed00[0x48]; // Thread, heap, delegate, and cancellation state.
    sead::Event* mpDoneEvents;
    sead::Event* mpLoadingEvents;
    al::AudioSystemInfo* mpAudioSystemInfo;
    bool mFastLoad;
};
static_assert(sizeof(AssetLoadingThread) == 0x68);
} // namespace rc
