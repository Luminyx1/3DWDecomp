#include "Enemy/SamboSnowHat.hpp"
#include "Library/ActorUtil.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"

namespace {
NERVE_DECL(SamboSnowHat, Wait)
NERVE_DECL(SamboSnowHat, Blow)
NERVES_MAKE_NOSTRUCT(SamboSnowHat, Wait, Blow)
}

/** @brief Constructs a snow Pokey hat.
 * @param pName Actor name.
 */
SamboSnowHat::SamboSnowHat(const char* pName) : al::LiveActor(pName) {}

/** @brief Initializes the hat's model and dormant state.
 * @param rInfo Actor placement and scene information.
 */
void SamboSnowHat::init(const al::ActorInitInfo& rInfo) {
    al::initActorWithArchiveName(this, rInfo, "SamboSnowHat", nullptr);
    al::initNerve(this, &NrvSamboSnowHatWait, 0);
    makeActorDead();
}

/** @brief Activates the detached hat's falling state. */
void SamboSnowHat::startBlow() {
    al::setNerve(this, &NrvSamboSnowHatBlow);
    makeActorAppeared();
}

/** @brief Waits for the hat to be detached. */
void SamboSnowHat::exeWait() {}

/** @brief Falls under gravity and disappears on landing or after 120 frames. */
void SamboSnowHat::exeBlow() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "BlowDown");
    }
    al::addVelocityToGravity(this, 1.5f);
    if (al::isOnGround(this, 0, 0.0f)) {
        al::startHitReactionDisappear(this);
        kill();
    }
    if (al::isGreaterEqualStep(this, 120)) {
        al::startHitReactionDisappear(this);
        kill();
    }
}
