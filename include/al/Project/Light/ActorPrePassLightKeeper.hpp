#pragma once

#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class ByamlIter;
class LiveActor;
class LppLine;
class LppPoint;
class LppProj;
class LppProjOrtho;
class LppSpot;
class PrePassLightBase;

class ActorPrePassLightKeeper {
public:
    struct LightBaseInfo {
        void setPtr();
        void readIter(const ByamlIter& rIter);

        const char* mName = nullptr;
        const char* mLppLightType = nullptr;
        const char* mLppLightShaderFunc = nullptr;
        const char* mActorJointName = nullptr;
        sead::Vector3f mOffset = sead::Vector3f::zero;
        sead::Vector3f mRotateOffset = sead::Vector3f::zero;
        sead::Color4f mColor = sead::Color4f::cWhite;
        sead::Color4f mSpecularColor = sead::Color4f::cWhite;
        bool mIsEnableSpecular = false;
        bool mIsEnableSpecularColor = false;
        s32 mKillFrame = -1;
        s32 mAppearFrame = -1;
        bool mIsIndirectIllumination = false;
    };

    static_assert(sizeof(LightBaseInfo) == 0x68);

    struct UserColor {
        void setPtr();
        void readIter(const ByamlIter& rIter);

        const char* mName = "色を追加できます";
        sead::Color4f mColor = sead::Color4f::cWhite;
    };

    ActorPrePassLightKeeper(bool isIgnoreYaml);
    ~ActorPrePassLightKeeper();

    bool init(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter);
    void setupPrePassPointLightParam(LppPoint* pLight) const;
    void setupPrePassSpotLightParam(LppSpot* pLight) const;
    void setupPrePassLineLightParam(LppLine* pLight) const;
    void setupPrePassProjOrthoLightParam(LppProjOrtho* pLight) const;
    void setupPrePassProjLightParam(LppProj* pLight) const;

    void initLightNum(s32 num);
    void setLightBaseInfo(PrePassLightBase* pLight, const LightBaseInfo& rInfo);
    void initAfterPlacement();
    void appear(bool isHideModel);
    void requestKill();
    void hideModel();
    void updateHideModel(bool isHide);
    PrePassLightBase* getLightBase(const char* pName) const;
    PrePassLightBase* getLightBase(s32 index) const;
    const sead::Color4f& findUserColor(const char* pName) const;

    s32 getLightNum() const { return mLightBaseArray.size(); }

private:
    sead::PtrArray<PrePassLightBase> mLightBaseArray;
    sead::PtrArray<UserColor> mUserColorArray;
    LiveActor* mParentActor;
    bool mIsIgnorePrePassYaml;
    bool mIsIgnoreHideModel = false;
};
}  // namespace al

namespace alYamlMacroUtil {
class YamlParamGroup;
}

namespace np_LightCommon {
extern alYamlMacroUtil::YamlParamGroup LightCommon;
}
