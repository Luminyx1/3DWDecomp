#include "Project/Se/SeListenerPoserAdjustMiddlePos.hpp"

#include <math/seadVector.h>

namespace al {

namespace {
const sead::Vector2f cFovyRatioTable[] = {{20.0f, 0.85f}, {30.0f, 0.7f}, {45.0f, 0.5f}};
}

/**
 * @brief Creates a listener poser with an initial halfway-to-target ratio.
 * @param rName Poser name used for selection.
 * @param rGroupName Group name used to select matching listener parameters.
 */
SeListenerPoserAdjustMiddlePos::SeListenerPoserAdjustMiddlePos(const sead::SafeString& rName,
                                                               const sead::SafeString& rGroupName)
    : SeListenerPoserMiddlePos(rName, rGroupName, 0.5f) {}

/**
 * @brief Interpolates the listener ratio from vertical field of view and calculates its pose.
 * @param pMtx Non-null output listener matrix.
 * @param pPos Non-null output listener position.
 * @param rParam Camera parameters, including vertical field of view in degrees.
 */
void SeListenerPoserAdjustMiddlePos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                      const ISeListenerParam& rParam) {
    f32 fovy = rParam.getFovyDegree();

    if (mPrevFovyDegree != fovy) {
        f32 ratio;

        if (fovy <= cFovyRatioTable[0].x) {
            ratio = cFovyRatioTable[0].y;
        } else {
            s32 i = 1;
            const sead::Vector2f* pPrev = cFovyRatioTable;
            const sead::Vector2f* pNext = cFovyRatioTable + 1;

            for (; i < 3; i++, pPrev++, pNext++) {
                if (fovy <= cFovyRatioTable[i].x) {
                    break;
                }
            }

            if (i < 3) {
                const sead::Vector2f& rPrev = *pPrev;
                const sead::Vector2f& rNext = cFovyRatioTable[i];
                ratio = rPrev.y + (fovy - rPrev.x) / (pNext->x - rPrev.x) * (rNext.y - rPrev.y);
            } else {
                ratio = cFovyRatioTable[2].y;
            }
        }

        setBaseToMiddleRatio(ratio);
        mPrevFovyDegree = fovy;
    }

    SeListenerPoserMiddlePos::calcListenerPose(pMtx, pPos, rParam);
}

} // namespace al
