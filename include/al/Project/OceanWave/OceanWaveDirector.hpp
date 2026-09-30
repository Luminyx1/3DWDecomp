#pragma once

#include <math/seadVector.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class OceanWaveInfo;

class OceanWaveDirector : public ISceneObj, public LiveActor, public IUseExecutor {
public:
    OceanWaveDirector(const char* pName);

    virtual void hide();
    virtual void show();
    void execute() override;
    virtual void createWave(const LiveActor* pActor, const OceanWaveInfo* pInfo);
    void kill() override;
    virtual f32 getY(const sead::Vector3f& rPos);
    virtual bool isInInk(const sead::Vector3f& rPos);
    void draw() const override;
    ~OceanWaveDirector() override;
};
}  // namespace al
