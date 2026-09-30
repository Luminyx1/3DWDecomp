#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
struct ActorInitInfo;

struct AnimScaleParam {
    AnimScaleParam();

    AnimScaleParam(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, s32 h, f32 i, f32 j, f32 k,
                   f32 l);

    f32 _0 = 0.2f;
    f32 _4 = 0.91f;
    f32 _8 = 0.2f;
    f32 _c = 1.8f;
    f32 _10 = 0.06f;
    f32 _14 = 0.12f;
    f32 _18 = 0.91f;
    s32 _1c = 20;
    f32 _20 = 0.25f;
    f32 _24 = 0.9f;
    f32 _28 = 5.2f;
    f32 _2c = 0.05f;
};

static_assert(sizeof(AnimScaleParam) == 0x30);

class AnimScaleController : public NerveExecutor {
public:
    AnimScaleController(const AnimScaleParam* pParam);

    void setAnimScaleParam(const AnimScaleParam* pParam);
    void startAnim();
    void startVibration();
    void startHitReaction();
    void startAndSetScaleVelocityY(f32 velocity);
    void startAndAddScaleVelocityY(f32 velocity);
    void startCrush();
    void stopAnim();
    void stopAndReset();
    void resetScale();
    void stopAndSetScale(const sead::Vector3f& rScale);
    void setScaleVelocityY(f32 velocity);
    void addScaleVelocityY(f32 velocity);
    void exeStop();
    void exeAnim();
    void updateScale(f32 stiffness, f32 damping);
    bool tryStop();
    void exeVibration();
    void exeHitReaction();
    void exeCrush();
    bool isHitReaction(s32 step) const;
    void setOriginalScale(const sead::Vector3f& rScale);
    void update();

    const sead::Vector3f& getScale() { return mScale; }

private:
    void updateScaleXZ();

    const AnimScaleParam* mParam;
    sead::Vector3f mAnimScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mOriginalScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    f32 mScaleVelocityY = 0.0f;
};

static_assert(sizeof(AnimScaleController) == 0x40);
}  // namespace al
