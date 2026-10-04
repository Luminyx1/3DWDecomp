#include <nn/atk/detail/SoundArchiveManager.h>
#include <nn/atk/atk_SoundArchive.h>

namespace nn::atk::detail {
/** @brief Initializes an empty archive manager without taking archive ownership. */
SoundArchiveManager::SoundArchiveManager()
    : mMainArchive(nullptr), mMainDataManager(nullptr), mTargetArchive(nullptr), mTargetDataManager(nullptr),
      mParametersHook(nullptr) {}

/** @brief Destroys the manager without finalizing externally owned archives. */
SoundArchiveManager::~SoundArchiveManager() = default;

/**
 * @brief Selects the main archive and clears previously registered addon containers.
 * @param pArchive Main archive to select; must outlive the manager's use of it.
 * @param pDataManager Resource manager associated with the main archive.
 */
void SoundArchiveManager::Initialize(const SoundArchive* pArchive, const SoundDataManager* pDataManager) {
    mMainArchive = pArchive;
    mMainDataManager = pDataManager;
    mContainerList.clear();
    mTargetArchive = mMainArchive;
    mTargetDataManager = mMainDataManager;
}

/** @brief Unlinks addon containers and releases the manager's archive references. */
void SoundArchiveManager::Finalize() {
    mTargetArchive = nullptr;
    mTargetDataManager = nullptr;
    mContainerList.clear();
    mParametersHook = nullptr;
    mMainArchive = nullptr;
    mMainDataManager = nullptr;
}

/**
 * @brief Appends an addon archive container to the lookup list.
 * @param rContainer Initialized, unlinked container that must outlive its registration.
 */
void SoundArchiveManager::Add(AddonSoundArchiveContainer& rContainer) {
    mContainerList.push_back(rContainer);
}

/**
 * @brief Removes an addon container if it belongs to this manager.
 * @param rContainer Container to locate by object identity.
 */
void SoundArchiveManager::Remove(AddonSoundArchiveContainer& rContainer) {
    for (auto it = mContainerList.begin(); it != mContainerList.end(); ++it) {
        if (&*it == &rContainer) {
            mContainerList.erase(it);
            return;
        }
    }
}

/**
 * @brief Selects an addon archive by name, falling back to the main archive.
 * @param pName Null-terminated addon name, or nullptr to select the main archive.
 */
void SoundArchiveManager::ChangeTargetArchive(const char* pName) {
    mTargetArchive = mMainArchive;
    mTargetDataManager = mMainDataManager;
    if (pName != nullptr) {
        for (auto& rContainer : mContainerList) {
            if (rContainer.IsSameName(pName)) {
                mTargetArchive = rContainer.GetSoundArchive();
                mTargetDataManager = rContainer.GetSoundDataManager();
                return;
            }
        }
    }
}

/** @brief Checks every registered archive. @return True if the main archive and all addons are available. */
bool SoundArchiveManager::IsAvailable() const {
    if (mMainArchive == nullptr) {
        return false;
    }
    bool mainAvailable = mMainArchive->IsAvailable();
    bool addonsAvailable = true;
    for (const auto& rContainer : mContainerList) {
        addonsAvailable = rContainer.GetSoundArchive()->IsAvailable() && addonsAvailable;
    }
    return mainAvailable && addonsAvailable;
}

/**
 * @brief Finds an addon archive by its registered name.
 * @param pName Null-terminated name, or nullptr for no addon.
 * @return Matching archive, or nullptr if no name matches.
 */
const AddonSoundArchive* SoundArchiveManager::GetAddonSoundArchive(const char* pName) const {
    return FindAddon(pName, &AddonSoundArchiveContainer::GetSoundArchive);
}

/**
 * @brief Finds the resource manager of a named addon archive.
 * @param pName Null-terminated name, or nullptr for no addon.
 * @return Matching resource manager, or nullptr if no name matches.
 */
const SoundDataManager* SoundArchiveManager::GetAddonSoundDataManager(const char* pName) const {
    return FindAddon(pName, &AddonSoundArchiveContainer::GetSoundDataManager);
}

/**
 * @brief Gets an addon container in registration order.
 * @param index Zero-based index; must be less than the registered container count.
 * @return Container at the requested index.
 */
const AddonSoundArchiveContainer* SoundArchiveManager::GetAddonSoundArchiveContainer(int index) const {
    auto it = mContainerList.begin();
    for (int i = 0; i < index; ++i) {
        ++it;
    }
    return &*it;
}

/**
 * @brief Gets a mutable addon container in registration order.
 * @param index Zero-based index; must be less than the registered container count.
 * @return Container at the requested index.
 */
AddonSoundArchiveContainer* SoundArchiveManager::GetAddonSoundArchiveContainer(int index) {
    auto it = mContainerList.begin();
    for (int i = 0; i < index; ++i) {
        ++it;
    }
    return &*it;
}

/**
 * @brief Installs a common parameter hook on the main archive and all registered addons.
 * @param pHook Parameter override provider, or nullptr to clear overrides; manager must be initialized.
 */
void SoundArchiveManager::SetParametersHook(SoundArchiveParametersHook* pHook) {
    mMainArchive->SetParametersHook(pHook);
    for (const auto& rContainer : mContainerList) {
        rContainer.GetSoundArchive()->SetParametersHook(pHook);
    }
    mParametersHook = pHook;
}

/** @brief Gets the common archive override hook. @return Current hook, or nullptr if none is installed. */
SoundArchiveParametersHook* SoundArchiveManager::GetParametersHook() const { return mParametersHook; }
} // namespace nn::atk::detail
