#pragma once

#include <nn/types.h>

namespace nn::atk {
class SoundHandle;

class SoundStartable {
public:
    class StartResult {
    public:
        enum ResultCode {
            ResultCode_Success = 0,
            ResultCode_ErrorInvalidStreamFilePath = 21,
            ResultCode_ErrorUser = 128,
            ResultCode_ErrorUnknown = 255
        };

        StartResult() : m_Code(ResultCode_ErrorUnknown) {}
        explicit StartResult(ResultCode code) : m_Code(code) {}

        bool IsSuccess() const { return m_Code == ResultCode_Success; }
        ResultCode GetCode() const { return m_Code; }

    private:
        ResultCode m_Code;
    };

    struct StartInfo;

    virtual ~SoundStartable() {}

    StartResult StartSound(SoundHandle* pHandle, u32 soundId, const StartInfo* pStartInfo = nullptr);
    StartResult StartSound(SoundHandle* pHandle, const char* pSoundName,
                           const StartInfo* pStartInfo = nullptr);
    StartResult HoldSound(SoundHandle* pHandle, u32 soundId, const StartInfo* pStartInfo = nullptr);
    StartResult HoldSound(SoundHandle* pHandle, const char* pSoundName,
                          const StartInfo* pStartInfo = nullptr);

private:
    virtual StartResult detail_SetupSound(SoundHandle* pHandle, u32 soundId, bool holdFlag,
                                          const char* pSoundArchiveName,
                                          const StartInfo* pStartInfo) = 0;
    virtual u32 detail_GetItemId(const char* pString) = 0;
    virtual u32 detail_GetItemId(const char* pString, const char* pSoundArchiveName) = 0;
};
}  // namespace nn::atk
