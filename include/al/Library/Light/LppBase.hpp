#pragma once

#include <basis/seadTypes.h>
#include <container/seadTList.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace al {
class ActorInitInfo;
class GraphicsSystemInfo;
class MtxConnector;
class PrePassLightKeeper;
class ShadowDirector;

class PrePassLightBase : public NerveExecutor,
                         public sead::TListNode<PrePassLightBase*>,
                         public IUseCollision {
public:
    virtual ~PrePassLightBase();
    virtual void init(const ActorInitInfo& rInfo);
    virtual void execute() = 0;
    virtual void declareLpp() = 0;
    virtual void calcClippingInfo(sead::Vector3f* pPos, f32* pRadius) = 0;
    virtual void trySetupShadow(s32, PrePassLightKeeper* pKeeper, ShadowDirector* pDirector);
    virtual s32 getLightType() const = 0;
    CollisionDirector* getCollisionDirector() const override;

    void appear();
    void requestAppearByUser(s32 step);
    void requestKillByUser(s32 step);
    void requestKillCore(s32 step);
    void requestKill();
    void requestKillDirect();
    const sead::Color4f& getColor() const;
    void requestUserColor(const sead::Color4f& rColor);
    void requestUserSpecularColor(const sead::Color4f& rColor);

    bool isActive() const { return isLinked(); }

    GraphicsSystemInfo* mGraphicsSystemInfo;
    CollisionDirector* mCollisionDirector;
    const char* mName;
    MtxConnector* mMtxConnector;
    sead::Vector3f mOffset;
    sead::Vector3f mRotateOffsetDegree;
    sead::Color4f mColor;
    sead::Color4f mSpecularColor;
    bool _90;
    bool mIsEnableSpecular;
    bool mIsEnableSpecularColor;
    s32 mLightShaderFunc;
    bool mIsUserColor;
    sead::Color4f mUserColor;
    bool mIsUserSpecularColor;
    sead::Color4f mUserSpecularColor;
    bool mIsKillByUser;
    sead::Color4f mCurrentColor;
    sead::Color4f mCurrentSpecularColor;
    s32 mAppearStep;
    s32 mAppearFrame;
    s32 mKillStep;
    s32 mKillFrame;
    bool mIsIndirectIllumination;
    u8 _f8[0x108 - 0xf8];
};

static_assert(sizeof(PrePassLightBase) == 0x108);

enum LppLightType : s32 {
    LppLightType_Line = 0,
    LppLightType_Point = 1,
    LppLightType_Spot = 2,
    LppLightType_Proj = 3,
    LppLightType_ProjOrtho = 4,
};

class LppPointParam {
public:
    static constexpr s32 cLightType = LppLightType_Point;

    f32 mRadius;
};

class LppLineParam {
public:
    static constexpr s32 cLightType = LppLightType_Line;
};

class LppSpotParam {
public:
    static constexpr s32 cLightType = LppLightType_Spot;

    f32 mDegree;
    f32 mLength;
    u8 _8[0x1c - 0x8];
    f32 mCurrentLength;
    u8 _20[0x24 - 0x20];
    s32 mShadowType;
    f32 mShadowParam;
    u8 _2c[0x30 - 0x2c];
    bool mIsStrikeCollision;
};

class LppProjParam {
public:
    static constexpr s32 cLightType = LppLightType_Proj;

    f32 mNear;
    f32 mFar;
    f32 mFovyDegree;
    u8 _c[0x178 - 0xc];
    s32 mShadowType;
    f32 mShadowParam;
};

class LppProjOrthoParam {
public:
    static constexpr s32 cLightType = LppLightType_ProjOrtho;

    void calcSizeXZ(f32* pSizeX, f32* pSizeZ) const;

    f32 mNear;
    u8 _4[0xc - 0x4];
    f32 mFarRate;
    u8 _10[0x178 - 0x10];
    s32 mShadowType;
    u8 _17c[0x180 - 0x17c];
    f32 mShadowParam;
};

template <typename T>
class PrePassLight : public PrePassLightBase {
public:
    s32 getLightType() const override { return T::cLightType; }

    T mParam;
};
}  // namespace al
