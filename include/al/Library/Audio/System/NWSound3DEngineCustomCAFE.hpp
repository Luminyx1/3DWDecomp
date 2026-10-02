#pragma once

#include <nn/atk/atk_Sound3DEngine.h>

namespace al {
class NWSound3DEngineCustomCAFE : public nn::atk::Sound3DEngine {
public:
    NWSound3DEngineCustomCAFE();

    void UpdateAmbientParam(nn::atk::SoundAmbientParam* pOutValue,
                            const nn::atk::Sound3DManager* pManager,
                            const nn::atk::Sound3DParam* pParam, u32 soundId,
                            u32 updateFlag) override;
};
}  // namespace al
