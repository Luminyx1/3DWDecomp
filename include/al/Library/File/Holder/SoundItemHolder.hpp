#pragma once

#include <container/seadBuffer.h>

namespace al {
class IAudioResourceLoader;
class SoundItemEntry;

class SoundItemHolder {
public:
    SoundItemHolder();
    SoundItemEntry* addNewLoadRequestEntry(u32 itemId, u32 unk, IAudioResourceLoader* pLoader);
    SoundItemEntry* tryFindEntry(u32 itemId, IAudioResourceLoader* pLoader);
    void waitLoadDoneAll();
    void clearEntry();

    sead::Buffer<SoundItemEntry> mSoundItemEntries;
    s32 mSize = 0;
};
}  // namespace al
