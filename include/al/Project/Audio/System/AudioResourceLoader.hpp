#pragma once

#include <basis/seadTypes.h>
#include <container/seadRingBuffer.h>
#include <prim/seadSafeString.h>

namespace al {
class IAudioResourceLoader {
public:
    virtual ~IAudioResourceLoader() = default;
    virtual bool loadSoundItem(u32 id, u32 loadFlag) = 0;
    virtual bool isLoadedSoundItem(u32 id) = 0;
    virtual u32 convertFixLoadBankId(u32 id) const { return id; }
};

class IAudioHeapController {
public:
    virtual ~IAudioHeapController() = default;
    virtual s32 saveHeapState() = 0;
    virtual void loadHeapState(s32 level) = 0;
    virtual s32 getCurrentHeapStateLevel() = 0;
    virtual u64 getSoundResourceHeapFreeSize() = 0;
};

class AudioResourceLayer : public IAudioResourceLoader {
public:
    AudioResourceLayer(IAudioResourceLoader* pLoader, s32 itemNum);

    void setData(const sead::SafeString& rName);
    void addLoadedSoundItemId(u32 id);
    bool loadSoundItem(u32 id, u32 loadFlag) override;
    bool isLoadedSoundItem(u32 id) override;

    const sead::SafeString& getName() const { return mName; }

private:
    sead::SafeString mName;
    IAudioResourceLoader* mLoader;
    sead::RingBuffer<u32> mLoadedItemIds;
};
static_assert(sizeof(AudioResourceLayer) == 0x38);
}  // namespace al
