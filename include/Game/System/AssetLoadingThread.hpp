#pragma once
#include <basis/seadTypes.h>
#include <thread/seadEvent.h>

namespace al {
class AudioSystemInfo;
} // namespace al

namespace sead {
class DelegateThread;
class Heap;
class Thread;
} // namespace sead

namespace rc {
class AssetLoadingThread {
  public:
    /// Groups of stationed resources loaded in the background.
    enum LOAD_TYPE : unsigned int {
        LOAD_TYPE_SHARED,
        LOAD_TYPE_PRE_3D_WORLD_TITLE,
        LOAD_TYPE_IN_3D_WORLD_TITLE,
        LOAD_TYPE_SINGLE_MODE,
        LOAD_TYPE_NUM
    };

    static AssetLoadingThread* createInstance(sead::Heap* pHeap);
    static void deleteInstance();

    AssetLoadingThread(sead::Heap* pHeap);
    ~AssetLoadingThread();

    void threadMsgReceive(sead::Thread* pThread, s64 msg);
    void startLoad(LOAD_TYPE type);
    void waitLoadDone(LOAD_TYPE type);
    bool isLoading(LOAD_TYPE type);
    void waitAllLoadingDone();
    bool isAllLoadingDone();
    bool isLoadDone(LOAD_TYPE type);
    bool isLoadOrLoading(LOAD_TYPE type);
    void disableFastLoad(bool disable);
    void cancelAndDelete3DWorldStationed();
    void cancelAndDeleteLoad(LOAD_TYPE type);
    void cancelAndDeleteSingleModeStationed();

    /**
     * @brief Set the audio system info used while loading assets.
     * @param pInfo The audio system info.
     */
    void setAudioSystemInfo(al::AudioSystemInfo* pInfo) { mpAudioSystemInfo = pInfo; }

    static AssetLoadingThread* sInstance;

  private:
    u8 mUnreconstructed00[0x20];
    sead::DelegateThread* mpThread;
    sead::Heap* mpParentHeap;
    sead::Heap** mpLoadHeaps;          // One heap per LOAD_TYPE.
    sead::Event* mpCancelEvent;        // Signaled to abort an in-flight load.
    sead::Event* mpThreadIdleEvent;    // Signaled when the thread finished a message.
    sead::Event* mpDoneEvents;         // One per LOAD_TYPE.
    sead::Event* mpLoadingEvents;      // One per LOAD_TYPE.
    al::AudioSystemInfo* mpAudioSystemInfo;
    bool mFastLoad;
};
static_assert(sizeof(AssetLoadingThread) == 0x68);
} // namespace rc
