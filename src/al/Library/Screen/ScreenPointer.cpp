#include "Library/Screen/ScreenPointer.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Screen/ScreenPointDirector.hpp"

namespace al {
/**
 * Creates a screen pointer.
 * @param rInfo actor init info
 * @param pHost actor owning the pointer
 * @param pPos pointer position
 */
ScreenPointer::ScreenPointer(const ActorInitInfo& rInfo, const LiveActor* pHost,
                             const sead::Vector3f* pPos)
    : mHost(pHost), mPos(pPos) {
    mHitInfos.allocBuffer(0x400, nullptr);
    mDirector = rInfo.mScreenPointerDirector;
}

/**
 * Collects the targets hit by a segment.
 * @param rStart segment start
 * @param rEnd segment end
 * @return whether a target was hit
 */
bool ScreenPointer::hitCheckSegment(const sead::Vector3f& rStart, const sead::Vector3f& rEnd) {
    return mDirector->hitCheckSegment(&mHitInfos, 0x400, rStart, rEnd);
}

/**
 * Collects the targets hit by a circle on screen.
 * @param rPos circle center on screen
 * @param radius circle radius
 * @return whether a target was hit
 */
bool ScreenPointer::hitCheckScreenCircle(const sead::Vector2f& rPos, f32 radius) {
    return mDirector->hitCheckScreenCircle(&mHitInfos, 0x400, rPos, radius);
}
}  // namespace al
