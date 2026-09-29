#include "Project/Se/ISeListenerParam.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
namespace {
struct FovyRatioPoint {
    f32 fovyDegree;
    f32 ratio;
};

const s32 cFovyRatioTableSize = 3;
}  // namespace

/**
 * @brief Constructs a middle-position listener poser whose ratio adapts to the camera's field of view.
 * @param rName The name of the poser.
 * @param rUnused Unused second name.
 */
SeListenerPoserAdjustMiddlePos::SeListenerPoserAdjustMiddlePos(const sead::SafeString& rName,
                                                               const sead::SafeString& rUnused)
    : SeListenerPoserMiddlePos(rName, rUnused, 0.5f) {}

/**
 * @brief Updates the camera-to-target ratio from the field of view, then computes the middle pose.
 * @param pMtx Output listener view matrix.
 * @param pPos Output listener position.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListenerPoserAdjustMiddlePos::calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos,
                                                      const ISeListenerParam& rParam) {
    const FovyRatioPoint cFovyRatioTable[cFovyRatioTableSize] = {{20.0f, 0.85f}, {30.0f, 0.7f}, {45.0f, 0.5f}};
    f32 fovy = rParam.getFovyDegree();
    if (mLastFovyDegree != fovy) {
        s32 index = 0;
        for (; index < cFovyRatioTableSize; index++) {
            if (fovy <= cFovyRatioTable[index].fovyDegree) {
                break;
            }
        }

        f32 ratio;
        if (index == 0) {
            ratio = cFovyRatioTable[0].ratio;
        } else if (index == cFovyRatioTableSize) {
            ratio = cFovyRatioTable[cFovyRatioTableSize - 1].ratio;
        } else {
            const FovyRatioPoint& prev = cFovyRatioTable[index - 1];
            const FovyRatioPoint& next = cFovyRatioTable[index];
            f32 rate = (fovy - prev.fovyDegree) / (next.fovyDegree - prev.fovyDegree);
            ratio = prev.ratio + rate * (next.ratio - prev.ratio);
        }
        setBaseToMiddleRatio(ratio);
        mLastFovyDegree = fovy;
    }
    SeListenerPoserMiddlePos::calcListenerPose(pMtx, pPos, rParam);
}
}  // namespace al
