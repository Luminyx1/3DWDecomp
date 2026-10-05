#pragma once

#include <nn/atk/atk_BasicSound.h>
#include <nn/atk/atk_Sound3DListener.h>
#include <nn/atk/atk_InstancePool.h>
#include <nn/atk/atk_Sound3DEngine.h>

namespace nn::atk {
class Sound3DEngine;
class SoundArchive;

class Sound3DManager : public detail::BasicSound::AmbientParamUpdateCallback,
                       public detail::BasicSound::AmbientArgAllocator {
  public:
    typedef util::IntrusiveList<
        Sound3DListener, util::IntrusiveListMemberNodeTraits<Sound3DListener, &Sound3DListener::m_LinkNode>>
        ListenerList;

    Sound3DManager();
    virtual ~Sound3DManager();

    size_t GetRequiredMemSize(const SoundArchive* pArchive);
    bool Initialize(const SoundArchive* pArchive, void* pBuffer, size_t size);
    bool InitializeWithMoreSoundArchive(const SoundArchive* pArchive);
    bool Finalize();
    void SetEngine(Sound3DEngine* pEngine);

    /** @brief Registers a spatial listener. @param pListener Unregistered, non-null listener. */
    void AddListener(Sound3DListener* pListener) { m_ListenerList.push_back(*pListener); }
    /** @brief Detaches a spatial listener. @param pListener Non-null listener registered with this manager.
     */
    void RemoveListener(Sound3DListener* pListener) {
        m_ListenerList.erase(m_ListenerList.iterator_to(*pListener));
    }
    /** @brief Gets the registered listeners. @return List owned by this manager. */
    const ListenerList& GetListenerList() const { return m_ListenerList; }

    /** @brief Sets the distance priority limit. @param maxPriorityReduction Maximum priority reduction. */
    void SetMaxPriorityReduction(int maxPriorityReduction) { m_MaxPriorityReduction = maxPriorityReduction; }
    /** @brief Gets the distance priority limit. @return Maximum priority reduction. */
    int GetMaxPriorityReduction() const { return m_MaxPriorityReduction; }
    /** @brief Sets spatial panning strength. @param panRange Scale applied to the calculated pan. */
    void SetPanRange(f32 panRange) { m_PanRange = panRange; }
    /** @brief Gets spatial panning strength. @return Scale applied to the calculated pan. */
    f32 GetPanRange() const { return m_PanRange; }
    /** @brief Sets the Doppler reference speed. @param sonicVelocity Sound propagation speed in scene units.
     */
    void SetSonicVelocity(f32 sonicVelocity) { m_SonicVelocity = sonicVelocity; }
    /** @brief Gets the Doppler reference speed. @return Sound propagation speed in scene units. */
    f32 GetSonicVelocity() const { return m_SonicVelocity; }
    void SetBiquadFilterType(int type);
    /** @brief Gets the spatial filter preset. @return Preset identifier, or -1 if disabled. */
    int GetBiquadFilterType() const { return m_BiquadFilterType; }

  private:
    void detail_UpdateAmbientParam(const void* pArg, u32 soundId, SoundAmbientParam* pParam) override;
    int detail_GetAmbientPriority(const void* pArg, u32 soundId) override;
    void* detail_AllocAmbientArg(size_t size) override;
    void detail_FreeAmbientArg(void* pArg, const detail::BasicSound* pSound) override;
    detail::PoolImpl mPool;
    ListenerList m_ListenerList;
    detail::ISound3DEngine* m_pSound3DEngine;
    s32 m_MaxPriorityReduction;
    f32 m_PanRange;
    f32 m_SonicVelocity;
    int m_BiquadFilterType;
    size_t mRemainingSize;
    bool mInitialized;
};
static_assert(sizeof(Sound3DManager) == 0x60);
} // namespace nn::atk
