#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPartsActorKeeper.hpp"
#include "Library/Layout/LayoutUtil.hpp"

namespace al {
/**
 * Reinitializes the shaders of a layout actor and all of its parts actors.
 * @param pActor layout actor
 */
void reinitializeShaders(LayoutActor* pActor) {
    if (LayoutKeeper* keeper = pActor->getLayoutKeeper()) {
        keeper->reinitializeShader();
    }

    LayoutPartsActorKeeper* partsKeeper = pActor->getLayoutPartsActorKeeper();

    if (!partsKeeper) {
        return;
    }

    s32 partsNum = partsKeeper->getPartsActorNum();

    for (s32 i = 0; i < partsNum; i++) {
        if (LayoutActor* parts = partsKeeper->getPartsActor(i)) {
            reinitializeShaders(parts);
        }
    }
}
}  // namespace al
