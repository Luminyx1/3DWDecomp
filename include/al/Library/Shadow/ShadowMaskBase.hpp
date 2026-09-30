#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Shadow/ShadowEnum.hpp"

namespace al {
class ByamlIter;
class LiveActor;
class MtxConnector;

AL_TEXT_ENUM(ShadowMaskDrawCategory,
             "ブロック,アイテム,地形オブジェ,敵,プレイヤー,ライトスケール,地形オブジェAO,敵AO,"
             "プレイヤーAO,ライトスケール(軽量),全体AO,地形オブジェAOSO,地形と水オブジェAOSO,"
             "敵AOSO,プレイヤーAOSO,全体AOSO,プレイヤー装飾",
             Block, Item, MapObj, Enemy, Player, LightScale, MapObjAO, EnemyAO, PlayerAO,
             LightScaleLight, AllAO, MapObjAOSO, MapObjAndWaterAOSO, EnemyAOSO, PlayerAOSO, AllAOSO,
             PlayerDecoration)

AL_TEXT_ENUM(ShadowMaskType, "無し,球,シリンダー,キューブ,楕円球投影,補間キューブ投影", None,
             Sphere, Cylinder, Cube, CastOvalCylinder, CastInterpolateCube)

class ShadowMaskBase {
public:
    ShadowMaskBase(const char* pName);

    virtual ~ShadowMaskBase() {}

    virtual void declare(ShadowMaskDrawCategory category) = 0;
    virtual void update() = 0;
    virtual void initAfterPlacement();
    virtual void calcShadowMatrix(sead::Matrix34f* pMtx);
    virtual void createMtxConnector();
    virtual void readParam(const ByamlIter& rIter);
    virtual void updateMulti() = 0;
    virtual void addMulti() = 0;
    virtual ShadowMaskType getShadowMaskType() const = 0;

    const sead::Color4f& getColor() const;
    void setColor(sead::Color4f color);
    f32 getShadowIntensity() const;
    void setHost(const LiveActor* pHost);

    const LiveActor* getHost() const { return mHost; }

    MtxConnector* getMtxConnector() const { return mMtxConnector; }

    ShadowMaskDrawCategory getDrawCategory() const { return mDrawCategory; }

    bool isValid() const { return mIsValid; }

    bool isHide() const { return mIsHide; }

    bool isIgnoreHide() const { return mIsIgnoreHide; }

    const LiveActor* mHost;
    MtxConnector* mMtxConnector;
    sead::Vector3f mOffset;
    sead::Color4f mColor;
    sead::Vector3f mDropDir;
    f32 mDropLength;
    sead::Vector3f mUp;
    const char* mName;
    bool mIsApplyShadowIntensityUser;
    u8 mShadowIntensityUser;
    bool mIsFollowHostScale;
    bool mIsIgnoreHide;
    ShadowMaskDrawCategory mDrawCategory;
    bool mIsShadowFixed;
    const char* mActorJointName;
    bool mIsRegistered;
    bool mIsValid;
    bool mIsHide;
    sead::Matrix34f mShadowMtx;
    sead::FixedSafeString<32> mSetHeightEvenTargetName;
    ShadowMaskBase* mHeightEvenTarget;
    bool _e8;
    bool mIsIgnoreHostAlpha;
    bool _ea;
};

static_assert(sizeof(ShadowMaskBase) == 0xf0);

}  // namespace al
