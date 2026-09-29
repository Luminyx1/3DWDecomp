#include "Library/Obj/EffectObjFunction.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
    namespace EffectObjFunction {
        /**
         * @brief Initializes an effect object actor using its placement object name.
         * @param pActor The actor to initialize.
         * @param rInfo The actor init info.
         */
        void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo) {
            const char* objectName = nullptr;
            getObjectName(&objectName, rInfo);
            initActorEffectObj(pActor, rInfo, objectName);
        }

        /**
         * @brief Initializes an effect object actor, with or without an archive, and its effect keeper.
         * @param pActor The actor to initialize.
         * @param rInfo The actor init info.
         * @param pName The object name used for the archive and effect keeper.
         */
        void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName) {
            StringTmp<128> archivePath("ObjectData/%s", pName);
            if (isExistArchive(archivePath)) {
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
            initActorEffectKeeper(pActor, rInfo, pName, true);
        }
    }  // namespace EffectObjFunction
}  // namespace al
