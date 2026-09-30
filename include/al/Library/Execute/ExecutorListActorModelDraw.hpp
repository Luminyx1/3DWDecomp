#pragma once

#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
class LiveActor;
class ModelDrawerBase;
struct ExecuteSystemInitInfo;

class ExecutorListActorModelDrawBase : public ExecutorListBase {
public:
    ExecutorListActorModelDrawBase(const char* pListName, s32 capacity, const char* pGroupName,
                                   const ExecuteSystemInitInfo& rInfo);
    ~ExecutorListActorModelDrawBase() override;

    void executeList() const override;
    bool isActive() const override { return mDrawerNum > 0; }
    virtual ModelDrawerBase* createDrawer(const char* pName) const = 0;

    void registerActorModel(LiveActor* pActor);
    void createList();

    s32 mDrawerNumMax;
    s32 mDrawerNum = 0;
    ModelDrawerBase** mDrawers;
};

class ExecutorListActorModelDrawForward : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForward(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawForwardCharacter : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForwardCharacter(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawForwardCharacterOpt : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForwardCharacterOpt(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawForwardCharacterXlu : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForwardCharacterXlu(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawForwardXlu : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForwardXlu(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawForwardPostEffectMask : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawForwardPostEffectMask(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawCubeMap : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawCubeMap(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferred : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferred(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredCharacter : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredCharacter(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredPlayer : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredPlayer(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredSSD : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredSSD(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawHeightMap : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawHeightMap(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredCharacterOpa : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredCharacterOpa(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredOpa : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredOpa(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredEcho : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredEcho(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredSilhouette : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredSilhouette(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredSilhouetteRide : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredSilhouetteRide(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDepthShadow : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDepthShadow(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDepthOnly : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDepthOnly(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDepthForce : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDepthForce(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDepthForceOpt : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDepthForceOpt(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawInvincible : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawInvincible(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredSky : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredSky(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};

class ExecutorListActorModelDrawDeferredFootPrint : public ExecutorListActorModelDrawBase {
public:
    ExecutorListActorModelDrawDeferredFootPrint(const char* pListName, s32 capacity, const char* pGroupName,
        const ExecuteSystemInitInfo& rInfo);
    ModelDrawerBase* createDrawer(const char* pName) const override;
};
}  // namespace al
