#pragma once

#include "Project/File/FileEntryBase.hpp"

namespace al {
namespace AudioConst {
extern const s32 SOUND_ID_INVALID;
}

class IAudioResourceLoader {
public:
    virtual void func_0() = 0;
    virtual void func_8() = 0;
    virtual bool tryLoad(u32 itemId, u32 unk) = 0;
};

class SoundItemEntry : public FileEntryBase {
public:
    SoundItemEntry();
    void load() override;
    void setLoadRequestInfo(u32 itemId, u32 unk, IAudioResourceLoader* pLoader);
    bool isLoadSuccess() const;
    u32 getSoundItemId() const;
    void clear();

    IAudioResourceLoader* getAudioResourceLoader() const { return mResourceLoader; }

    s32 mItemId = AudioConst::SOUND_ID_INVALID;
    s32 _bc = -1;
    IAudioResourceLoader* mResourceLoader = nullptr;
    volatile bool mIsLoadSuccess = false;
};
}  // namespace al
