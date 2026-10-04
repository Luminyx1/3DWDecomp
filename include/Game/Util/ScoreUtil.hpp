#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class HitSensor;
    class IUseSceneObjHolder;
    class LiveActor;
    class ScreenPointer;
    class SensorMsg;
};  // namespace al

namespace rc {
    void addScore(const al::LiveActor*, al::HitSensor*, f32, s32);
    void addScore(const al::LiveActor*, al::HitSensor*, const sead::Vector3f&, s32);
    void addScore(const al::LiveActor*, al::HitSensor*, const sead::Vector3f*,
                  const sead::Vector3f&, s32);
    void addScoreByFactor(const al::LiveActor*, al::HitSensor*, const char*, f32, s32);
    void addScoreByFactor(const al::LiveActor*, al::HitSensor*, const char*,
                          const sead::Vector3f&, s32);
    void addScoreByFactor(const al::LiveActor*, al::HitSensor*, const char*,
                          const sead::Vector3f*, const sead::Vector3f&, s32);
    void addScoreCombo(const al::LiveActor*, al::HitSensor*, const al::SensorMsg*, f32);
    void addScoreCombo(const al::LiveActor*, al::HitSensor*, const al::SensorMsg*,
                       const sead::Vector3f&);
    void addScoreCombo(const al::LiveActor*, al::HitSensor*, const al::SensorMsg*,
                       const sead::Vector3f*, const sead::Vector3f&);
    void addScoreComboByFactor(const al::LiveActor*, al::HitSensor*, const char*,
                               const al::SensorMsg*, f32);
    void addScore(const al::LiveActor*, al::ScreenPointer*, f32, s32);
    void addScore(const al::LiveActor*, al::ScreenPointer*, const sead::Vector3f&, s32);
    void addScoreByFactor(const al::LiveActor*, al::ScreenPointer*, const char*, f32, s32);
    void addScoreByFactor(const al::LiveActor*, al::ScreenPointer*, const char*,
                          const sead::Vector3f&, s32);
    void addScoreCombo(const al::LiveActor*, al::ScreenPointer*, const al::SensorMsg*, f32);
    void addScoreCombo(const al::LiveActor*, al::ScreenPointer*, const al::SensorMsg*,
                       const sead::Vector3f&);
    void addScoreBySystem(const al::LiveActor*, al::HitSensor*, const char*, s32);
    void addScoreBySystem(const al::LiveActor*, al::HitSensor*, const char*,
                          const sead::Vector3f&, s32);
};  // namespace rc

namespace ScoreFunction {
    void popUpPlayerOneUp(const al::LiveActor*, f32);
    void popUpPlayerUp(const al::LiveActor*, s32, f32);
    void popUpPlayerOneUp(const al::LiveActor*, const sead::Vector3f&);
    void popUpPlayerUp(const al::LiveActor*, s32, const sead::Vector3f&);
    s32 getPlayerScore(const al::IUseSceneObjHolder*, s32);
};  // namespace ScoreFunction
