#pragma once

#include <basis/seadTypes.h>

#include "Library/File/FileEntryBase.hpp"

namespace al {
namespace AudioConst {
extern const u32 SOUND_ID_INVALID;
}  // namespace AudioConst

class IAudioResourceLoader {
public:
    virtual void func_0() = 0;
    virtual void func_8() = 0;
    virtual bool tryLoad(u32, u32) = 0;
};

class SoundItemEntry : public FileEntryBase {
public:
    SoundItemEntry();

    void load() override;
    void setLoadRequestInfo(u32, u32, IAudioResourceLoader*);
    bool isLoadSuccess() const;
    u32 getSoundItemId() const;
    void clear();

    u32 mItemId = AudioConst::SOUND_ID_INVALID;       // _B8
    s32 _BC = -1;                                     // _BC
    IAudioResourceLoader* mResourceLoader = nullptr;  // _C0
    volatile bool mIsLoadSuccess = false;             // _C8
};
}  // namespace al
