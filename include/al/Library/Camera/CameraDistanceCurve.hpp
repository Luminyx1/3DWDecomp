#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;

struct CameraDistanceAtPoint {
    f32 distance;
    f32 angleV;
};

class CameraDistanceCurve {
public:
    CameraDistanceCurve(const char* pName, const CameraDistanceAtPoint* pPoints, s32 pointNum);

    static const CameraDistanceCurve* getDefaultCurve();
    static const CameraDistanceCurve* findOrDefaultCurve(const ByamlIter& rIter);
    static const CameraDistanceCurve* getRocketFlowerCurve();
    static const CameraDistanceCurve* getGiantWanderBossCurve();
    static const CameraDistanceCurve* getKoopaShellCurve();
    static const CameraDistanceCurve* getTRexPatrolCurve();

    f32 calcDistance(f32 angleV) const;

    const char* getName() const { return mName; }

private:
    const char* mName;
    const CameraDistanceAtPoint* mPoints;
    s32 mPointNum;
};

static_assert(sizeof(CameraDistanceCurve) == 0x18);
}  // namespace al
