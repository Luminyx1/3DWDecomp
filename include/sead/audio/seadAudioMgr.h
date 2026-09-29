#pragma once

#include "audio/seadAudioSubsetBase.h"
#include "container/seadOffsetList.h"
#include "heap/seadDisposer.h"
#include "hostio/seadHostIONode.h"

namespace sead {
class AudioPlayer;
class AudioResetter;
class AudioResourceLoader;
class AudioSettingParameter;
class AudioSystem;

namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio

class AudioMgr : public hostio::Node {
    SEAD_SINGLETON_DISPOSER(AudioMgr)

public:
    AudioMgr();
    virtual ~AudioMgr();

    void prepare(AudioSettingParameter* pParam, Heap* pHeap, s32 addonArchiveCount);
    void exit();
    void appendAudioSubset(AudioSubsetBase* pSubset);
    bool removeAudioSubset(AudioSubsetBase* pSubset);
    void calc();
    void initHostIO(hostio::Node* pNode);
    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

    AudioSystem* getAudioSystem() const { return mAudioSystem; }
    AudioResetter* getResetter() const { return mResetter; }
    AudioPlayer* getPlayer() const { return mPlayer; }
    AudioResourceLoader* getResourceLoader() const { return mResourceLoader; }
    const OffsetList<AudioSubsetBase>& getSubsetList() const { return mSubsetList; }

private:
    AudioSystem* mAudioSystem = nullptr;
    AudioResetter* mResetter = nullptr;
    AudioPlayer* mPlayer = nullptr;
    AudioResourceLoader* mResourceLoader = nullptr;
    OffsetList<AudioSubsetBase> mSubsetList;
    Heap* mHeap = nullptr;
    bool mIsPrepared = false;
    bool mIsAudioSystemOwned = false;
    bool mIsResetterOwned = false;
    bool mIsPlayerOwned = false;
};
static_assert(sizeof(AudioMgr) == 0x70);
}  // namespace sead
