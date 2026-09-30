#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioSystemInfo;
class ModelKeeper;
class SeEmitterInfo;
class SeSource;
class SeSourcePose;

class SeEmitter {
public:
    SeEmitter(AudioSystemInfo* pInfo, const SeEmitterInfo* pEmitterInfo, const ModelKeeper* pModelKeeper,
              SeSourcePose* pPose, bool isUseModel);

    bool update();
    void activate();
    const char* getName() const;

    SeSource* getSeSource() const { return mSeSource; }

private:
    SeSource* mSeSource = nullptr;
    const SeEmitterInfo* mEmitterInfo;
    s32 mSilentFrames = 15;
};
static_assert(sizeof(SeEmitter) == 0x18);
}  // namespace al
