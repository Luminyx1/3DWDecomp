#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioEffectDataBase;
class SeadAudioPlayer;

class SeEffectController {
public:
    SeEffectController();

    void init(SeadAudioPlayer* pSePlayer, SeadAudioPlayer* pBgmPlayer, const AudioEffectDataBase* pDataBase);
    void createEffectUnit(const char* pName);
    void finalize();
    void update();
    void changeEffect(const char* pName);

    const char* getCurEffectName() const {
        if (mCurEffectInfo == nullptr) {
            return nullptr;
        }

        return *mCurEffectInfo;
    }

private:
    u8 _0[0x30];
    const char** mCurEffectInfo;
    u8 _38[0x30];
};
}  // namespace al
