#include "Library/Obj/EffectObjFunction.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al::EffectObjFunction {
/**
 * Initializes an effect object using its object name as archive name.
 * @param pActor actor
 * @param rInfo actor init info
 */
void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo) {
    const char* objectName = nullptr;
    getObjectName(&objectName, rInfo);
    initActorEffectObj(pActor, rInfo, objectName);
}

/**
 * Initializes an effect object, with its archive if it exists.
 * @param pActor actor
 * @param rInfo actor init info
 * @param pArchiveName archive and effect keeper name
 */
void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName) {
    if (isExistArchive(StringTmp<128>("ObjectData/%s", pArchiveName))) {
        initActor(pActor, rInfo);
    } else {
        initActorSceneInfo(pActor, rInfo);
        initActorPoseTQSV(pActor);
        initActorSRT(pActor, rInfo);
        initActorClipping(pActor, rInfo);
        initGroupClipping(pActor, rInfo, 64);
        setClippingInfo(pActor, 1000.0f, nullptr);
        initStageSwitch(pActor, rInfo);
        initExecutorUpdate(pActor, rInfo, "エフェクトオブジェ");
        alPlacementFunction::isEnableGroupClipping(rInfo);
    }

    initActorEffectKeeper(pActor, rInfo, pArchiveName, true);
}
}  // namespace al::EffectObjFunction
