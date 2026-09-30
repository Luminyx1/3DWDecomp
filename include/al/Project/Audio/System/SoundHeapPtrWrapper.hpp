#pragma once

#include <basis/seadTypes.h>

namespace nn::atk {
class SoundArchive;
class SoundDataManager;
}  // namespace nn::atk

namespace sead {
class AudioSoundHeapNin;
}

namespace al {
class IAudioResourceInfoAccessor;

namespace AudioConst {
extern const u32 SOUND_ID_INVALID;
}

class SeadAudioSoundHeapPtrWrapper {
public:
    SeadAudioSoundHeapPtrWrapper();

    void setSoundHeap(sead::AudioSoundHeapNin* pHeap);
    s32 saveHeapState();
    void loadHeapState(s32 level);
    void* alloc(size_t size);
    s32 getCurrentHeapStateLevel() const;
    size_t getHeapSize() const;
    size_t getHeapFreeSize() const;
    void dumpHeap(nn::atk::SoundDataManager* pMgr, nn::atk::SoundArchive* pArchive);

private:
    sead::AudioSoundHeapNin* mHeap;
};

class SoundNameUtil {
public:
    SoundNameUtil() = default;
    virtual ~SoundNameUtil();

    IAudioResourceInfoAccessor* getAccessor() const { return mAccessor; }
    void setAccessor(IAudioResourceInfoAccessor* pAccessor) { mAccessor = pAccessor; }

private:
    IAudioResourceInfoAccessor* mAccessor = nullptr;
};
}  // namespace al

namespace alSoundNameFunction {
void initializeNameUtil(al::IAudioResourceInfoAccessor* pAccessor, bool isBgm);
}

namespace alSoundNameUtil {
bool isExistItemId(u32 id, bool isBgm);
bool isExistItemName(const char* pName, bool isBgm);
u32 getSoundType(u32 id, bool isBgm);
u32 getItemType(u32 id);
const char* getSoundName(u32 id, bool isBgm);
u32 getSoundId(const char* pName, bool isBgm);
bool tryGetSoundId(u32* pId, const char* pName, bool isBgm);
}  // namespace alSoundNameUtil
