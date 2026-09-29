#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>

namespace al {
class IAudioResourceLoader;
class SoundItemEntry;

class SoundItemHolder {
public:
    SoundItemHolder();

    SoundItemEntry* addNewLoadRequestEntry(u32, u32, IAudioResourceLoader*);
    SoundItemEntry* tryFindEntry(u32, IAudioResourceLoader*);
    void waitLoadDoneAll();
    void clearEntry();

    sead::Buffer<SoundItemEntry> mEntries;  // _0
    s32 mNumEntries = 0;                    // _10
};
}  // namespace al
