#pragma once

#include <basis/seadTypes.h>
#include <container/seadTList.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>

#include "common/aglTextureData.h"

#include "Library/Nerve/NerveExecutor.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace agl {
class TextureSampler;
}  // namespace agl

namespace sead {
class Random;
}  // namespace sead

namespace al {
class ActorInitInfo;
class CollisionDirector;
class GraphicsSystemInfo;
class LiveActor;
class MtxConnector;
class PrePassLightKeeper;
class ShadowDirector;

// clang-format off
SEAD_ENUM(LppLightType,無し,点光源,スポットライト,投影,正射影,ラインライト)
SEAD_ENUM(LppLightShaderFunc,通常,裏面)
// clang-format on

class PrePassLightBase : public NerveExecutor,
                         public sead::TListNode<PrePassLightBase*>,
                         public IUseCollision {
public:
    PrePassLightBase(const char* pName);

    virtual void init(const ActorInitInfo& rInfo);
    virtual void execute() = 0;
    virtual void declareLpp() = 0;
    virtual void calcClippingInfo(sead::Vector3f* pPos, f32* pRadius) = 0;
    virtual void trySetupShadow(s32 view, PrePassLightKeeper* pKeeper, ShadowDirector* pDirector);
    virtual s32 getLightType() const = 0;

    CollisionDirector* getCollisionDirector() const override { return mCollisionDirector; }

    void appear();
    void requestAppearByUser(s32 step);
    void requestKillByUser(s32 step);
    void requestKillCore(s32 step);
    void requestKill();
    void requestKillDirect();
    const sead::Color4f& getColor() const;
    void requestUserColor(const sead::Color4f& rColor);
    void requestUserSpecularColor(const sead::Color4f& rColor);
    void calcColor(sead::Color4f* pColor, f32 rate) const;
    void calcConnectorInfo(sead::Vector3f* pTrans, sead::Quatf* pQuat,
                           sead::Vector3f* pScale) const;
    void resetRequest();
    void exeAppear();
    void exeWait();
    void exeKill();
    void exeDead();
    f32 calcRandomRate(bool isPaused);

    bool isActive() const { return isLinked(); }

    GraphicsSystemInfo* getGraphicsSystemInfo() const { return mGraphicsSystemInfo; }

    sead::Vector3f calcRotateOffsetRadian() const {
        return mRotateOffsetDegree * sead::Mathf::deg2rad(1.0f);
    }

    GraphicsSystemInfo* mGraphicsSystemInfo = nullptr;
    CollisionDirector* mCollisionDirector = nullptr;
    const char* mName;
    MtxConnector* mMtxConnector;
    sead::Vector3f mOffset = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mRotateOffsetDegree = {0.0f, 0.0f, 0.0f};
    sead::Color4f mColor = sead::Color4f::cWhite;
    sead::Color4f mSpecularColor = sead::Color4f::cGray;
    bool _90 = false;
    bool mIsEnableSpecular = false;
    bool mIsEnableSpecularColor = false;
    s32 mLightShaderFunc = 0;
    bool mIsUserColor = false;
    sead::Color4f mUserColor = sead::Color4f::cWhite;
    bool mIsUserSpecularColor = false;
    sead::Color4f mUserSpecularColor = sead::Color4f::cWhite;
    bool mIsKillByUser = false;
    sead::Color4f mCurrentColor = sead::Color4f::cBlack;
    sead::Color4f mCurrentColorStart = sead::Color4f::cBlack;
    s32 mAppearStep = -1;
    s32 mAppearFrame = -1;
    s32 mKillStep = -1;
    s32 mKillFrame = -1;
    bool mIsIndirectIllumination = false;
    sead::Random* mRandom;
    f32 mRandomCeil = 0.0f;
    f32 mRandomRate = 1.0f;
};

static_assert(sizeof(PrePassLightBase) == 0x108);

class LppTexInfo {
public:
    ~LppTexInfo();

    void initByInfo(const ActorInitInfo& rInfo);
    void updateTexOffset();

    agl::TextureData mTextureData;
    agl::TextureSampler* mSampler = nullptr;
    bool mIsEnableTexOption = false;
    sead::Vector2f mTexOffset = {0.0f, 0.0f};
    sead::Vector2f mTexScale = {1.0f, 1.0f};
    const char* mTextureBaseName = nullptr;
    sead::Vector2f mTexMoveSpeed;
    sead::Vector2f mTexMoveSpeedFrame;
};

static_assert(sizeof(LppTexInfo) == 0x160);

class LppPointParam {
public:
    static constexpr s32 cLightType = LppLightType::点光源;

    void initByInfo(const ActorInitInfo& rInfo);

    f32 mRadius = 100.0f;
    f32 mDampPower = 1.0f;
    f32 mSpecExpansion = 0.0f;
};

class LppLineParam {
public:
    static constexpr s32 cLightType = LppLightType::ラインライト;

    void initByInfo(const ActorInitInfo& rInfo);

    f32 mRadius = 100.0f;
    f32 mLength = 100.0f;
    f32 mSpecExpansion = 0.0f;
};

class LppSpotParam {
public:
    static constexpr s32 cLightType = LppLightType::スポットライト;

    void initByInfo(const ActorInitInfo& rInfo);

    f32 mDegree = 30.0f;
    f32 mLength = 500.0f;
    f32 mAngleDamp = 1.0f;
    f32 mDistDamp;
    f32 mSpecExpansion = 0.0f;
    bool mIsEnableCollisionCheck = false;
    f32 mCollisionOffset = 0.0f;
    f32 mCurrentLength = 500.0f;
    f32 mLengthChangeRate = 0.25f;
    s32 mShadowType = 0;
    f32 mPcf = 1.5f;
    s32 mShadowIndex = -1;
    bool mIsHitCollision = false;
    sead::Vector3f mHitPos = sead::Vector3f::zero;
};

static_assert(sizeof(LppSpotParam) == 0x40);

class LppProjParam {
public:
    static constexpr s32 cLightType = LppLightType::投影;

    void initByInfo(const ActorInitInfo& rInfo);

    f32 mNear = 100.0f;
    f32 mFar = 500.0f;
    f32 mFovyDegree = 20.0f;
    f32 mAspect = 1.0f;
    f32 mDistDamp = 1.0f;
    LppTexInfo mTexInfo;
    s32 mShadowType = 0;
    f32 mPcf = 1.5f;
    s32 mShadowIndex = -1;
};

static_assert(sizeof(LppProjParam) == 0x188);

class LppProjOrthoParam {
public:
    static constexpr s32 cLightType = LppLightType::正射影;

    void initByInfo(const ActorInitInfo& rInfo);
    void calcSizeXZ(f32* pSizeX, f32* pSizeZ) const;

    f32 mNear = 100.0f;
    f32 mDistDamp = 1.0f;
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    LppTexInfo mTexInfo;
    s32 mShadowType = 0;
    bool mIsUseParentYRotation = false;
    f32 mPcf = 1.5f;
    s32 mShadowIndex = -1;
};

static_assert(sizeof(LppProjOrthoParam) == 0x188);

}  // namespace al

namespace LppFunction {
void initLppActor(al::LiveActor* pActor, al::PrePassLightBase* pLight, sead::Matrix34f* pMtx,
                  const al::ActorInitInfo& rInfo);
void recalculateRotation(al::LiveActor* pActor, const al::ActorInitInfo& rInfo);

template <typename T>
void declareLpp(const T& rParam, const al::PrePassLightBase& rLight, s32 num);

template <typename T>
void requestLpp(T* pParam, al::PrePassLightBase* pLight);

void requestLppPoint(al::LppPointParam* pParam, al::PrePassLightBase* pLight,
                     const sead::Vector3f& rPos);

template <typename T>
void calcClippingInfoLpp(T* pParam, al::PrePassLightBase* pLight, sead::Vector3f* pPos,
                         f32* pRadius);

template <typename T>
void trySetupShadowLpp(al::PrePassLightKeeper* pKeeper, al::ShadowDirector* pDirector, T* pParam,
                       s32 view);
}  // namespace LppFunction

namespace PrePassLightPlacementFuncImpl {
void setClippingInfoImpl(al::LiveActor* pActor, f32 radius, const sead::Vector3f* pPos);
void makeMtxSRT(sead::Matrix34f* pMtx, const al::LiveActor* pActor);
}  // namespace PrePassLightPlacementFuncImpl
