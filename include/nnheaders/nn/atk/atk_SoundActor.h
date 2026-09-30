#pragma once

#include <nn/atk/atk_BasicSound.h>
#include <nn/atk/atk_SoundStartable.h>

namespace nn::atk {
class SoundArchivePlayer;

class SoundActor : public SoundStartable {
public:
    SoundActor();
    ~SoundActor() override;

    void Initialize(SoundArchivePlayer* pSoundArchivePlayer);
    void StopAllSound(int fadeFrames);
    void PauseAllSound(bool flag, int fadeFrames);
    int GetPlayingSoundCount(int actorPlayerId) const;

    virtual StartResult SetupSound(SoundHandle* pHandle, u32 soundId, const StartInfo* pStartInfo,
                                   void* pSetupArg);
    virtual StartResult SetupSound(SoundHandle* pHandle, u32 soundId, const char* pSoundArchiveName,
                                   const StartInfo* pStartInfo, void* pSetupArg);

private:
    virtual StartResult detail_SetupSoundWithAmbientInfo(SoundHandle* pHandle, u32 soundId,
                                                         const char* pSoundArchiveName,
                                                         const StartInfo* pStartInfo,
                                                         detail::BasicSound::AmbientInfo* pAmbientInfo,
                                                         void* pSetupArg);
    StartResult detail_SetupSound(SoundHandle* pHandle, u32 soundId, bool holdFlag,
                                  const char* pSoundArchiveName, const StartInfo* pStartInfo) override;
    u32 detail_GetItemId(const char* pString) override;
    u32 detail_GetItemId(const char* pString, const char* pSoundArchiveName) override;

    u8 _8[0xcc - 0x8];
    bool m_IsInitialized;
    bool m_IsFinalized;
};
static_assert(sizeof(SoundActor) == 0xd0);
}  // namespace nn::atk
