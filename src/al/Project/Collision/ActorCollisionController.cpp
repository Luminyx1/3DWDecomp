#include "Library/Collision/ActorCollisionController.hpp"

#include "Library/LiveActor/ActorCollisionFunction.hpp"

namespace al {
/**
 * Constructs a controller that remembers the current collider size of an actor.
 * @param pActor actor to control
 */
ActorCollisionController::ActorCollisionController(LiveActor* pActor) : mActor(pActor) {
    mRadius = getColliderRadius(mActor);
    mOffsetY = getColliderOffsetY(mActor);
}

/**
 * Sets the collider radius and stops restoring the original size.
 * @param radius new radius
 */
void ActorCollisionController::setColliderRadius(f32 radius) {
    mDelay = -1;
    al::setColliderRadius(mActor, radius);
}

/**
 * Sets the collider offset and stops restoring the original size.
 * @param offsetY new vertical offset
 */
void ActorCollisionController::setColliderOffsetY(f32 offsetY) {
    mDelay = -1;
    al::setColliderOffsetY(mActor, offsetY);
}

/**
 * Moves the collider size towards the original size.
 */
void ActorCollisionController::update() {
    if (mDelay <= 0) {
        return;
    }

    if (mDelay == 1) {
        resetToOrigin(mDelay);
        return;
    }

    f32 radius = getColliderRadius(mActor);
    f32 offsetY = getColliderOffsetY(mActor);
    f32 rate = 1.0f / mDelay;
    radius += (mRadius - radius) * rate;
    offsetY += (mOffsetY - offsetY) * rate;
    al::setColliderRadius(mActor, radius);
    al::setColliderOffsetY(mActor, offsetY);
    mDelay--;
}

/**
 * Restores the original collider size, immediately or over several frames.
 * @param delay number of frames to take
 */
void ActorCollisionController::resetToOrigin(s32 delay) {
    if (delay >= 2) {
        mDelay = delay;
        return;
    }

    al::setColliderRadius(mActor, mRadius);
    al::setColliderOffsetY(mActor, mOffsetY);
    mDelay = 0;
}
}  // namespace al
