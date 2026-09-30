#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {

struct CameraShakeParam {
    const char* name;
    s32 steps;
    f32 speed;
    f32 power;
    s32 direction;
    f32 multipleRate;
    bool isMultiple;
    bool isKeepMultiple;
    bool _1e;
    bool _1f;
};

static_assert(sizeof(CameraShakeParam) == 0x20);

class CameraShaker_RS : public NerveExecutor {
public:
    CameraShaker_RS();

    void update(const char* pLoopShakeName, bool isPaused);
    void startShakeByAction(const char* pShakeName, const char* pActionName,
                            const char* pActorName, s32 steps);
    void startShakeByName(const char* pShakeName, s32 steps);
    void startShakeByHitReaction(const char* pShakeName, const char* pReactionName,
                                 const char* pActorName, s32 steps);
    void cancelShake();
    void exeWait();
    void exeShake();
    void exeShakeMultiple();
    void exeShakeLoop();
    void startShakeByIndex(s32 index, s32 steps);

    const sead::Vector2f& getOffset() const { return mOffset; }

    f32 getRoll() const { return mRoll; }

private:
    sead::Vector2f mOffset = {0.0f, 0.0f};
    f32 mRoll = 0.0f;
    const CameraShakeParam* mShakeParam = nullptr;
    const CameraShakeParam* mLoopParam = nullptr;
    CameraShakeParam mCustomParam = {};
};

static_assert(sizeof(CameraShaker_RS) == 0x50);
}  // namespace al
