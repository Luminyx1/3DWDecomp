#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
class Viewport;
}  // namespace sead

namespace al {
class ActorInitInfo;
class Resource;
class AreaObjDirector;
class CollisionDirector;
class IUseAudioKeeper;
class PlayerHolder;
class SceneCameraInfo;

class CameraDirector {
public:
    CameraDirector(s32 viewNum, AreaObjDirector* pAreaObjDirector,
                   CollisionDirector* pCollisionDirector);

    void init(const PlayerHolder* pPlayerHolder);
    void initCameraCreator(s32 resourceNum, bool isStageOneResource);
    void setCameraResource(const Resource* pResource, s32 index);
    void initAudioKeeper(ActorInitInfo& rInfo);
    void setCameraAspect(const sead::Viewport* pMainViewport, const sead::Viewport* pSubViewport);
    void setStageName(const char* pStageName);
    void update(bool isPaused);
    s32 getCameraMode() const;
    void setCameraMode(s32 mode);
    void setSnapShotAudioKeeper(IUseAudioKeeper* pAudioKeeper);
    void validUserCameraControlByPortOnly(s32 port);

    SceneCameraInfo* getSceneCameraInfo() const { return mSceneCameraInfo; }
    sead::LookAtCamera* getLookAtCamera() const { return mLookAtCamera; }
    sead::PerspectiveProjection* getProjection() const { return mProjection; }
    void setMainPlayerIndex(s32 index) { mMainPlayerIndex = index; }
    void setReverseHorizontal(bool isReverse) { mIsReverseHorizontal = isReverse; }
    void setReverseVertical(bool isReverse) { mIsReverseVertical = isReverse; }

    u8 _0[0x8];
    sead::LookAtCamera* mLookAtCamera;
    u8 _10[0x8];
    sead::PerspectiveProjection* mProjection;
    u8 _20[0x8];
    SceneCameraInfo* mSceneCameraInfo;
    u8 _30[0xa0 - 0x30];
    sead::Matrix34f mMainViewMtx;  // 0xa0
    sead::Matrix34f mSubViewMtx;   // 0xd0
    u8 _100[0x15c - 0x100];
    bool mIsReverseHorizontal;  // 0x15c
    bool mIsReverseVertical;  // 0x15d
    u8 _15e[0x161 - 0x15e];
    bool mIsTitleFlag;  // 0x161, cleared by the title scene (meaning unknown)
    u8 _162[0x1c8 - 0x162];
    s32 mMainPlayerIndex;  // 0x1c8
    u8 _1cc[0x1f0 - 0x1cc];
};
}  // namespace al
