#include <nn/atk/atk_AddonSoundArchiveContainer.h>
#include <nn/util/util_StringUtil.h>

namespace nn::atk::detail {
namespace {
// text is scanned for a terminator within at most count bytes.
int BoundedLength(const char* text, int count) {
    int length = 0;
    for (; count > 0; --count) {
        if (!*text++) break;
        ++length;
    }

    return length;
}

// left and right are compared for at most count bytes, stopping at a terminator.
int CompareNames(const char* left, const char* right, int count) {
    unsigned char a = 0, b = 0;
    for (; count > 0; --count) {
        a = *left++;
        b = *right++;
        if (!a || a != b) break;
    }

    return a - b;
}
}

AddonSoundArchiveContainer::AddonSoundArchiveContainer()
    : mInitialized(false), mArchive(nullptr), mDataManager(nullptr), mAddTick(0) {}
AddonSoundArchiveContainer::~AddonSoundArchiveContainer() {
    mInitialized = false;
    mArchive = nullptr;
    mDataManager = nullptr;
}

// name identifies archive; manager provides access to its sound resources.
bool AddonSoundArchiveContainer::Initialize(const char* name, const AddonSoundArchive* archive, const SoundDataManager* manager) {
    nn::util::Strlcpy(mName, name, sizeof(mName));
    mArchive = archive;
    mDataManager = manager;
    mInitialized = true;
    return true;
}

void AddonSoundArchiveContainer::Finalize() {
    mInitialized = false;
    mName[0] = 0;
    mArchive = nullptr;
    mDataManager = nullptr;
}

// name is compared with the stored archive name, rejecting names of 64 bytes or more.
bool AddonSoundArchiveContainer::IsSameName(const char* name) const {
    if (BoundedLength(name, sizeof(mName)) == sizeof(mName)) return false;
    return CompareNames(name, mName, sizeof(mName)) == 0;
}

// tick records when this archive was added to the player.
void AddonSoundArchiveContainer::SetAddTick(const nn::os::Tick& tick) { mAddTick = tick; }
}
