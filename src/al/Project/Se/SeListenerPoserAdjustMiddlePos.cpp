#include "Project/Se/SeListenerPoserAdjustMiddlePos.hpp"

#include <math/seadVector.h>

namespace al {

namespace {
const sead::Vector2f cFovyRatioTable[] = {{20.0f, 0.85f}, {30.0f, 0.7f}, {45.0f, 0.5f}};
}

SeListenerPoserAdjustMiddlePos::SeListenerPoserAdjustMiddlePos(const sead::SafeString& rName,
                                                               const sead::SafeString& rGroupName)
    : SeListenerPoserMiddlePos(rName, rGroupName, 0.5f) {}

void SeListenerPoserAdjustMiddlePos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                      const ISeListenerParam& rParam) {
    f32 fovy = rParam.getFovyDegree();
    if (mPrevFovyDegree != fovy) {
        f32 ratio;
        if (fovy <= cFovyRatioTable[0].x) {
            ratio = cFovyRatioTable[0].y;
        } else {
            s32 i = 1;
            for (; i < 3; i++) {
                if (fovy <= cFovyRatioTable[i].x) {
                    break;
                }
            }

            if (i < 3) {
                const sead::Vector2f& prev = cFovyRatioTable[i - 1];
                const sead::Vector2f& next = cFovyRatioTable[i];
                ratio = prev.y + (fovy - prev.x) / (next.x - prev.x) * (next.y - prev.y);
            } else {
                ratio = cFovyRatioTable[2].y;
            }
        }

        setBaseToMiddleRatio(ratio);
        mPrevFovyDegree = fovy;
    }

    SeListenerPoserMiddlePos::calcListenerPose(pMtx, pPos, rParam);
}

}  // namespace al
