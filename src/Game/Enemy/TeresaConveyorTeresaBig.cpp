#include "Enemy/TeresaConveyorTeresaBig.hpp"
#include "Library/ActorUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Util/PlayerUtil.hpp"

namespace {
NERVE_DECL(TeresaConveyorTeresaBig, Wait)
NERVES_MAKE_NOSTRUCT(TeresaConveyorTeresaBig, Wait)
}

/** @brief Constructs the conveyor's large Boo.
 * @param pName Actor name.
 */
TeresaConveyorTeresaBig::TeresaConveyorTeresaBig(const char* pName) : al::LiveActor(pName) {}

/** @brief Initializes the large Boo with inactive hit sensors.
 * @param rInfo Actor placement and scene information.
 */
void TeresaConveyorTeresaBig::init(const al::ActorInitInfo& rInfo) {
    al::initActorWithArchiveName(this, rInfo, "TeresaBig", nullptr);
    al::initNerve(this, &NrvTeresaConveyorTeresaBigWait, 0);
    al::invalidateHitSensors(this);
    makeActorAppeared();
}

/** @brief Plays the idle animations and turns toward the nearest active player. */
void TeresaConveyorTeresaBig::exeWait() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        al::startAction(al::getSubActor(this, 0), "Wait");
    }
    auto* pPlayer = rc::tryFindNearestActivePlayerActorInSphere(this, 3000.0f);
    if (pPlayer) {
        al::turnToTarget(this, pPlayer, 0.55f);
    }
}
