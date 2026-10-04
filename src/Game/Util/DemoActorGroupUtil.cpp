#include "Util/DemoActorGroupUtil.hpp"
#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Sequence/DemoDirector.hpp"

namespace DemoActorGroupUtil {
/**
 * Registers an actor with every demo actor group it is linked to, using each group's shared label.
 * @param pActor actor to register
 * @param rInfo actor init info (provides the demo director)
 * @param rPlacementInfo placement info of the actor whose links are scanned
 */
void initDemoActorGroup(al::LiveActor* pActor, const al::ActorInitInfo& rInfo,
                        const al::PlacementInfo& rPlacementInfo) {
    if (al::calcLinkCountClassName(rPlacementInfo, "DemoActorGroup") == 0) {
        return;
    }

    al::DemoDirector* pDemoDirector = rInfo.getActorSceneInfo().demoDirector;

    al::PlacementInfo links;
    al::tryGetPlacementInfoByKey(&links, rPlacementInfo, "Links");
    s32 linkNum = al::getCountPlacementInfo(links);

    for (s32 i = 0; i < linkNum; i++) {
        al::PlacementInfo linkInfo;
        const char* pKeyName = nullptr;
        al::tryGetPlacementInfoAndKeyNameByIndex(&linkInfo, &pKeyName, links, i);

        al::PlacementInfo groupInfo;
        al::tryGetPlacementInfoByIndex(&groupInfo, linkInfo, 0);

        if (al::isClassName(groupInfo, "DemoActorGroup")) {
            const char* pSharedLabel = nullptr;

            if (al::tryGetStringArg(&pSharedLabel, groupInfo, "SharedLabel")) {
                pDemoDirector->registerActorWithGroup(pActor, pSharedLabel);
            }
        }
    }
}

/**
 * Adds a demo actor group to the demo director. Does nothing in this game.
 * @param pGroup demo actor group
 * @param pDemoDirector demo director
 */
void addDemoActorGroup(DemoActorGroup* pGroup, al::DemoDirector* pDemoDirector) {}
}  // namespace DemoActorGroupUtil
