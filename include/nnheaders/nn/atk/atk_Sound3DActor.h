#pragma once

#include <nn/atk/atk_SoundActor.h>
#include <nn/util/util_VectorApi.h>

namespace nn::atk {
class Sound3DManager;

class Sound3DActor : public SoundActor, public detail::BasicSound::AmbientArgUpdateCallback {
public:
    Sound3DActor();
    ~Sound3DActor() override;

    void Initialize(SoundArchivePlayer* pPlayer, Sound3DManager* pManager);
    void Finalize();
    void SetPosition(const util::Vector3fType& rPosition);
    void ResetPosition();
    void SetVelocity(const util::Vector3fType& rVelocity);

    const util::Vector3fType& GetPosition() const { return m_Position; }
    const util::Vector3fType& GetVelocity() const { return m_Velocity; }

    StartResult SetupSound(SoundHandle* pHandle, u32 soundId, const StartInfo* pStartInfo,
                           void* pSetupArg) override;
    StartResult SetupSound(SoundHandle* pHandle, u32 soundId, const char* pSoundArchiveName,
                           const StartInfo* pStartInfo, void* pSetupArg) override;

private:
    void detail_UpdateAmbientArg(void* pArg, const detail::BasicSound* pSound) override;

    Sound3DManager* m_p3dManager;
    SoundArchivePlayer* m_pArchivePlayer;
    u32 m_UserParam;
    util::Vector3fType m_Position;
    util::Vector3fType m_Velocity;
    bool m_ResetPositionFlag;
    bool m_IsInitialized;
    bool m_IsFinalized;
};
static_assert(sizeof(Sound3DActor) == 0x120);
}  // namespace nn::atk
