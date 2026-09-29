#include "Library/Screen/ScreenPointer.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Screen/ScreenPointDirector.hpp"

namespace al {

/**
 * @brief Constructs a pointer that can hit screen point targets.
 * @param rInfo The init info of the owning actor.
 * @param pActor The owning actor.
 * @param pPos The pointer position.
 */
ScreenPointer::ScreenPointer(const ActorInitInfo& rInfo, const LiveActor* pActor,
                             const sead::Vector3f* pPos)
    : mActor(pActor), mPos(pPos) {
    mHitInfoArray.allocBuffer(0x400, nullptr);
    mDirector = rInfo.mScreenPointerDirector;
}

/**
 * @brief Collects the targets hit by a segment.
 * @param rStart The segment start.
 * @param rEnd The segment end.
 * @return True if any target was hit.
 */
bool ScreenPointer::hitCheckSegment(const sead::Vector3f& rStart, const sead::Vector3f& rEnd) {
    return mDirector->hitCheckSegment(&mHitInfoArray, 0x400, rStart, rEnd);
}

/**
 * @brief Collects the targets hit by a circle on screen.
 * @param rPos The circle center in screen space.
 * @param radius The circle radius.
 * @return True if any target was hit.
 */
bool ScreenPointer::hitCheckScreenCircle(const sead::Vector2f& rPos, f32 radius) {
    return mDirector->hitCheckScreenCircle(&mHitInfoArray, 0x400, rPos, radius);
}

}  // namespace al
