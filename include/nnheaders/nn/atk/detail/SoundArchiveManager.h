#pragma once

#include <nn/atk/atk_AddonSoundArchiveContainer.h>

namespace nn::atk {
class SoundArchive;
class SoundDataManager;
namespace detail {
class SoundArchiveParametersHook;

class SoundArchiveManager {
  public:
    SoundArchiveManager();
    ~SoundArchiveManager();
    void Initialize(const SoundArchive* pArchive, const SoundDataManager* pDataManager);
    void Finalize();
    void Add(AddonSoundArchiveContainer& rContainer);
    void Remove(AddonSoundArchiveContainer& rContainer);
    void ChangeTargetArchive(const char* pName);
    bool IsAvailable() const;
    const AddonSoundArchive* GetAddonSoundArchive(const char* pName) const;
    const SoundDataManager* GetAddonSoundDataManager(const char* pName) const;
    const AddonSoundArchiveContainer* GetAddonSoundArchiveContainer(int index) const;
    AddonSoundArchiveContainer* GetAddonSoundArchiveContainer(int index);
    void SetParametersHook(SoundArchiveParametersHook* pHook);
    SoundArchiveParametersHook* GetParametersHook() const;

    /** @brief Counts registered addon archives. @return Number of linked containers. */
    int GetAddonSoundArchiveCount() const { return mContainerList.size(); }

  private:
    using ContainerTraits = nn::util::IntrusiveListMemberNodeTraits<AddonSoundArchiveContainer,
                                                                    &AddonSoundArchiveContainer::mNode>;
    using ContainerList = nn::util::IntrusiveList<AddonSoundArchiveContainer, ContainerTraits>;
    template <class T> using ContainerGetter = const T* (AddonSoundArchiveContainer::*)() const;

    /**
     * @brief Looks up one resource of an addon archive by name.
     * @tparam T Resource type returned by the container getter.
     * @param pName Null-terminated archive name, or nullptr for no addon.
     * @param getter Typed accessor for the resource to retrieve from a matching container.
     * @return Selected resource, or nullptr if no container matches.
     */
    template <class T> const T* FindAddon(const char* pName, ContainerGetter<T> getter) const {
        if (pName == nullptr) {
            return nullptr;
        }
        for (const auto& rContainer : mContainerList) {
            if (rContainer.IsSameName(pName)) {
                return (rContainer.*getter)();
            }
        }
        return nullptr;
    }

    const SoundArchive* mMainArchive;
    const SoundDataManager* mMainDataManager;
    ContainerList mContainerList;
    const SoundArchive* mTargetArchive;
    const SoundDataManager* mTargetDataManager;
    SoundArchiveParametersHook* mParametersHook;
};
static_assert(sizeof(SoundArchiveManager) == 0x38, "SoundArchiveManager size");
} // namespace detail
} // namespace nn::atk
