#pragma once
#include <nn/os.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
class AddonSoundArchive;
class SoundDataManager;
namespace detail {
class AddonSoundArchiveContainer {
public:
    AddonSoundArchiveContainer();
    ~AddonSoundArchiveContainer();
    bool Initialize(const char* name, const AddonSoundArchive* archive, const SoundDataManager* manager);
    void Finalize();
    bool IsSameName(const char* name) const;
    void SetAddTick(const nn::os::Tick& tick);
private:
    nn::util::IntrusiveListNode mNode;
    bool mInitialized;
    const AddonSoundArchive* mArchive;
    const SoundDataManager* mDataManager;
    char mName[64];
    nn::os::Tick mAddTick;
};
static_assert(sizeof(AddonSoundArchiveContainer) == 0x70, "AddonSoundArchiveContainer size");
}
}
