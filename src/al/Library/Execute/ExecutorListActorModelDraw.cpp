#include "Library/Execute/ExecutorListActorModelDraw.hpp"

#include <nvn/nvn_FuncPtrInline.h>

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
enum class EchoDrawCategory : u64 {};
enum class SilhouetteDrawCategory : u64 {};

class ModelDrawerForward : public ModelDrawerBase {
public:
    ModelDrawerForward(const char* pName, bool a, bool b, bool c, bool d);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x20];
};

class ModelDrawerCubeMap : public ModelDrawerBase {
public:
    ModelDrawerCubeMap(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x18];
};

class ModelDrawerDeferred : public ModelDrawerBase {
public:
    ModelDrawerDeferred(const char* pName, bool a, bool b);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x30];
};

class ModelDrawerDeferredPlayer : public ModelDrawerBase {
public:
    ModelDrawerDeferredPlayer(const char* pName, bool a, bool b);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x30];
};

class ModelDrawerDeferredExt : public ModelDrawerBase {
public:
    ModelDrawerDeferredExt(const char* pName, bool a, bool b);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x40];
};

class ModelDrawerHeightMap : public ModelDrawerBase {
public:
    ModelDrawerHeightMap(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerDeferredEcho : public ModelDrawerBase {
public:
    ModelDrawerDeferredEcho(const char* pName, EchoDrawCategory category);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerDeferredSilhouette : public ModelDrawerBase {
public:
    ModelDrawerDeferredSilhouette(const char* pName, SilhouetteDrawCategory category);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerDepthShadow : public ModelDrawerBase {
public:
    ModelDrawerDepthShadow(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x18];
};

class ModelDrawerDepthOnly : public ModelDrawerBase {
public:
    ModelDrawerDepthOnly(const char* pName, bool a, bool b);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerInvincible : public ModelDrawerBase {
public:
    ModelDrawerInvincible(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerDeferredSky : public ModelDrawerBase {
public:
    ModelDrawerDeferredSky(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

class ModelDrawerDeferredFootPrint : public ModelDrawerBase {
public:
    ModelDrawerDeferredFootPrint(const char* pName);

    void createTable() override;
    void draw() const override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    u8 _20[0x90];
};

static inline void pushDebugGroup(NVNcommandBuffer* pCmdBuf, const char* pName) {
    reinterpret_cast<void (*)(NVNcommandBuffer*, const char*)>(
        pfnc_nvnCommandBufferPushDebugGroup)(pCmdBuf, pName);
}

/**
 * Constructs an actor model draw executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawBase::ExecutorListActorModelDrawBase(const char* pListName,
                                                               s32 capacity,
                                                               const char* pGroupName,
                                                               const ExecuteSystemInitInfo& rInfo)
    : ExecutorListBase(pListName, pGroupName), mDrawerNumMax(capacity) {
    mDrawers = new ModelDrawerBase*[capacity];

    for (s32 i = 0; i < mDrawerNumMax; i++) {
        mDrawers[i] = nullptr;
    }
}

/**
 * Registers an actor's model to the drawer matching its model name, creating it if needed.
 * @param pActor The actor.
 */
void ExecutorListActorModelDrawBase::registerActorModel(LiveActor* pActor) {
    alModelCafe* model = pActor->getModelKeeper()->getModelCafe();
    const char* modelName = pActor->getModelKeeper()->getModelName();

    for (s32 i = 0; i < mDrawerNum; i++) {
        ModelDrawerBase* drawer = mDrawers[i];

        if (isEqualString(drawer->getName(), modelName)) {
            drawer->registerModel(model);
            pActor->getExecuteInfo()->addDrawer(drawer);
            return;
        }
    }

    ModelDrawerBase* drawer = createDrawer(modelName);
    drawer->setDrawInfo(pActor->getSceneInfo()->graphicsSystemInfo, model);
    drawer->registerModel(model);
    pActor->getExecuteInfo()->addDrawer(drawer);
    mDrawers[mDrawerNum] = drawer;
    mDrawerNum++;
}

/**
 * Creates the tables of all drawers.
 */
void ExecutorListActorModelDrawBase::createList() {
    for (s32 i = 0; i < mDrawerNum; i++) {
        mDrawers[i]->createTable();
    }
}

/**
 * Draws all drawers inside a debug group.
 */
void ExecutorListActorModelDrawBase::executeList() const {
    if (mDrawerNum <= 0) {
        return;
    }

    if (isEqualString(mListName, "シルエット[プレイヤー]")) {
        pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(),
                       "Render Silhouette Player");
    } else if (isEqualString(mListName, "シルエット[乗り物]")) {
        pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(),
                       "Render Silhouette Ride");
    } else {
        pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(),
                       "Actor Model");
    }

    for (s32 i = 0; i < mDrawerNum; i++) {
        mDrawers[i]->draw();
    }

    nvnCommandBufferPopDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer());
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForward::ExecutorListActorModelDrawForward(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForward::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, false, false, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForwardCharacter::ExecutorListActorModelDrawForwardCharacter(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForwardCharacter::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, true, false, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForwardCharacterOpt::ExecutorListActorModelDrawForwardCharacterOpt(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForwardCharacterOpt::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, true, false, false, true);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForwardCharacterXlu::ExecutorListActorModelDrawForwardCharacterXlu(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForwardCharacterXlu::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, true, true, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForwardXlu::ExecutorListActorModelDrawForwardXlu(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForwardXlu::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, false, true, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawForwardPostEffectMask::ExecutorListActorModelDrawForwardPostEffectMask(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawForwardPostEffectMask::createDrawer(const char* pName) const {
    return new ModelDrawerForward(pName, false, true, true, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawCubeMap::ExecutorListActorModelDrawCubeMap(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawCubeMap::createDrawer(const char* pName) const {
    return new ModelDrawerCubeMap(pName);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferred::ExecutorListActorModelDrawDeferred(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferred::createDrawer(const char* pName) const {
    return new ModelDrawerDeferred(pName, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredCharacter::ExecutorListActorModelDrawDeferredCharacter(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredCharacter::createDrawer(const char* pName) const {
    return new ModelDrawerDeferred(pName, true, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredPlayer::ExecutorListActorModelDrawDeferredPlayer(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredPlayer::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredPlayer(pName, true, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredSSD::ExecutorListActorModelDrawDeferredSSD(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredSSD::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredExt(pName, true, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawHeightMap::ExecutorListActorModelDrawHeightMap(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawHeightMap::createDrawer(const char* pName) const {
    return new ModelDrawerHeightMap(pName);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredCharacterOpa::ExecutorListActorModelDrawDeferredCharacterOpa(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredCharacterOpa::createDrawer(const char* pName) const {
    return new ModelDrawerDeferred(pName, true, true);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredOpa::ExecutorListActorModelDrawDeferredOpa(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredOpa::createDrawer(const char* pName) const {
    return new ModelDrawerDeferred(pName, false, true);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredEcho::ExecutorListActorModelDrawDeferredEcho(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredEcho::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredEcho(pName, static_cast<EchoDrawCategory>(0));
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredSilhouette::ExecutorListActorModelDrawDeferredSilhouette(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredSilhouette::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredSilhouette(pName, static_cast<SilhouetteDrawCategory>(0));
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredSilhouetteRide::ExecutorListActorModelDrawDeferredSilhouetteRide(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredSilhouetteRide::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredSilhouette(pName, static_cast<SilhouetteDrawCategory>(1));
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDepthShadow::ExecutorListActorModelDrawDepthShadow(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDepthShadow::createDrawer(const char* pName) const {
    return new ModelDrawerDepthShadow(pName);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDepthOnly::ExecutorListActorModelDrawDepthOnly(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDepthOnly::createDrawer(const char* pName) const {
    return new ModelDrawerDepthOnly(pName, false, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDepthForce::ExecutorListActorModelDrawDepthForce(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDepthForce::createDrawer(const char* pName) const {
    return new ModelDrawerDepthOnly(pName, true, false);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDepthForceOpt::ExecutorListActorModelDrawDepthForceOpt(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDepthForceOpt::createDrawer(const char* pName) const {
    return new ModelDrawerDepthOnly(pName, true, true);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawInvincible::ExecutorListActorModelDrawInvincible(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawInvincible::createDrawer(const char* pName) const {
    return new ModelDrawerInvincible(pName);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredSky::ExecutorListActorModelDrawDeferredSky(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredSky::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredSky(pName);
}

/**
 * Constructs the executor list.
 * @param pListName List name.
 * @param capacity Maximum number of drawers.
 * @param pGroupName Group name.
 * @param rInfo Execute system init info.
 */
ExecutorListActorModelDrawDeferredFootPrint::ExecutorListActorModelDrawDeferredFootPrint(const char* pListName, s32 capacity, const char* pGroupName,
    const ExecuteSystemInitInfo& rInfo)
    : ExecutorListActorModelDrawBase(pListName, capacity, pGroupName, rInfo) {}

/**
 * Creates the drawer of this list.
 * @param pName Drawer name.
 * @return The drawer.
 */
ModelDrawerBase* ExecutorListActorModelDrawDeferredFootPrint::createDrawer(const char* pName) const {
    return new ModelDrawerDeferredFootPrint(pName);
}

ExecutorListActorModelDrawBase::~ExecutorListActorModelDrawBase() {
    for (s32 i = 0; i < mDrawerNumMax; i++) {
        if (mDrawers[i] != nullptr) {
            delete mDrawers[i];
        }
    }
}
}  // namespace al
