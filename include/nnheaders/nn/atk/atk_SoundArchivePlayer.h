#pragma once

#include <nn/atk/atk_SoundArchive.h>
#include <nn/atk/atk_SoundPlayer.h>
#include <nn/atk/atk_SoundStartable.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
class SoundDataManager;

namespace detail {
class AddonSoundArchiveContainer {
public:
    util::IntrusiveListNode m_ElementLink;
};

template <class Element>
class IntrusiveList {
public:
    int Count() const { return m_ListImpl.size(); }

private:
    util::IntrusiveList<Element,
                        util::IntrusiveListMemberNodeTraits<Element, &Element::m_ElementLink>>
        m_ListImpl;
};

class SoundArchiveManager {
public:
    int GetAddonSoundArchiveCount() const { return m_ContainerList.Count(); }

private:
    const SoundArchive* m_pMainSoundArchive;
    const SoundDataManager* m_pMainSoundDataManager;
    IntrusiveList<AddonSoundArchiveContainer> m_ContainerList;
    const SoundArchive* m_pCurrentSoundArchive;
    const SoundDataManager* m_pCurrentSoundDataManager;
    void* m_pParametersHook;
};
static_assert(sizeof(SoundArchiveManager) == 0x38);
}  // namespace detail

class SoundArchivePlayer : public SoundStartable {
public:
    struct InitializeParam {
        InitializeParam()
            : pSoundArchive(nullptr), pSoundDataManager(nullptr), pSetupBuffer(nullptr),
              setupBufferSize(0), pStreamBuffer(nullptr), streamBufferSize(0),
              pStreamCacheBuffer(nullptr), streamCacheSize(0),
              enablePreparingStreamInstanceBufferFromSetupBuffer(true),
              pStreamInstanceBuffer(nullptr), streamInstanceBufferSize(0),
              userParamSizePerSound(0), addonSoundArchiveCount(0) {}

        const SoundArchive* pSoundArchive;
        const SoundDataManager* pSoundDataManager;
        void* pSetupBuffer;
        size_t setupBufferSize;
        void* pStreamBuffer;
        size_t streamBufferSize;
        void* pStreamCacheBuffer;
        size_t streamCacheSize;
        bool enablePreparingStreamInstanceBufferFromSetupBuffer;
        void* pStreamInstanceBuffer;
        size_t streamInstanceBufferSize;
        size_t userParamSizePerSound;
        int addonSoundArchiveCount;
    };
    static_assert(sizeof(InitializeParam) == 0x68);

    SoundArchivePlayer();
    ~SoundArchivePlayer() override;

    static size_t GetRequiredMemSize(const SoundArchive* pArchive, size_t userParamSizePerSound);
    static size_t GetRequiredMemSize(const InitializeParam& rParam);
    static size_t GetRequiredStreamCacheSize(const SoundArchive* pArchive, size_t cacheSizePerSound);
    size_t GetRequiredStreamBufferSize(const SoundArchive* pArchive) const;
    bool Initialize(const InitializeParam& rParam);
    void Finalize();
    bool IsAvailable() const;
    void Update();

    u32 GetSoundPlayerCount() const { return m_SoundPlayerCount; }
    SoundPlayer& GetSoundPlayer(SoundArchive::ItemId playerId);
    int GetAddonSoundArchiveCount() const { return m_SoundArchiveManager.GetAddonSoundArchiveCount(); }
    const AddonSoundArchive* GetAddonSoundArchive(int index) const;
    const char* GetAddonSoundArchiveName(int index) const;

protected:
    StartResult detail_SetupSound(SoundHandle* pHandle, u32 soundId, bool holdFlag,
                                  const char* pSoundArchiveName, const StartInfo* pStartInfo) override;

private:
    u32 detail_GetItemId(const char* pString) override;
    u32 detail_GetItemId(const char* pString, const char* pSoundArchiveName) override;

    detail::SoundArchiveManager m_SoundArchiveManager;
    u32 m_SoundPlayerCount;
    u8 _44[0x310 - 0x44];
};
static_assert(sizeof(SoundArchivePlayer) == 0x310);
}  // namespace nn::atk
