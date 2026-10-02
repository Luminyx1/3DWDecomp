#pragma once

#include <nn/atk/atk_BasicSound.h>
#include <nn/atk/atk_Sound3DListener.h>

namespace nn::atk {
class Sound3DEngine;
class SoundArchive;

class Sound3DManager {
public:
    typedef util::IntrusiveList<Sound3DListener,
                                util::IntrusiveListMemberNodeTraits<Sound3DListener, &Sound3DListener::m_LinkNode>>
        ListenerList;

    Sound3DManager();
    virtual ~Sound3DManager();

    size_t GetRequiredMemSize(const SoundArchive* pArchive);
    bool Initialize(const SoundArchive* pArchive, void* pBuffer, size_t size);
    void Finalize();
    void SetEngine(Sound3DEngine* pEngine);

    void AddListener(Sound3DListener* pListener) { m_ListenerList.push_back(*pListener); }
    void RemoveListener(Sound3DListener* pListener) {
        m_ListenerList.erase(m_ListenerList.iterator_to(*pListener));
    }
    const ListenerList& GetListenerList() const { return m_ListenerList; }

    void SetMaxPriorityReduction(int maxPriorityReduction) { m_MaxPriorityReduction = maxPriorityReduction; }
    int GetMaxPriorityReduction() const { return m_MaxPriorityReduction; }
    void SetPanRange(f32 panRange) { m_PanRange = panRange; }
    f32 GetPanRange() const { return m_PanRange; }
    void SetSonicVelocity(f32 sonicVelocity) { m_SonicVelocity = sonicVelocity; }
    f32 GetSonicVelocity() const { return m_SonicVelocity; }
    void SetBiquadFilterType(int type);
    int GetBiquadFilterType() const { return m_BiquadFilterType; }

private:
    void* _8;
    u8 _10[0x28 - 0x10];
    ListenerList m_ListenerList;
    void* m_pSound3DEngine;
    s32 m_MaxPriorityReduction;
    f32 m_PanRange;
    f32 m_SonicVelocity;
    int m_BiquadFilterType;
    u8 _50[0x60 - 0x50];
};
static_assert(sizeof(Sound3DManager) == 0x60);
}  // namespace nn::atk
