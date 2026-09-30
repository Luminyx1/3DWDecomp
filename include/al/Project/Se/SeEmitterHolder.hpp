#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class AudioSystemInfo;
class ModelKeeper;
class SeEmitter;
class SeEmitterInfo;
class SeSourcePose;

class SeEmitterHolder {
public:
    SeEmitterHolder(AudioSystemInfo* pInfo, const sead::SafeString& rName,
                    const AudioInfoList<SeEmitterInfo>* pEmitterInfoList, const ModelKeeper* pModelKeeper,
                    SeSourcePose* pPose, bool isUseModel);

    void update();
    SeEmitter* findEmitter(const char* pName) const;
    SeEmitter* getEmitter(s32 index) const;
    void resetVelocity();

    s32 getEmitterNum() const { return mEmitters.size(); }
    bool isActive() const { return mIsActive; }
    void setIsActive(bool isActive) { mIsActive = isActive; }

private:
    sead::PtrArray<SeEmitter> mEmitters;
    bool mIsActive;
};
}  // namespace al
