#pragma once

#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class ByamlIter;
class LiveActor;
class PrePassLightBase;

class ActorPrePassLightKeeper {
public:
    struct LightBaseInfo {
        void setPtr();

        const char* mName;
        const char* mLppLightType;
        const char* mLppLightShaderFunc;
        const char* mActorJointName;
        sead::Vector3f mOffset;
        sead::Vector3f mRotateOffset;
        sead::Color4f mColor;
        sead::Color4f mSpecularColor;
        bool mIsEnableSpecular;
        bool mIsEnableSpecularColor;
        s32 mKillFrame;
        s32 mAppearFrame;
        bool mIsIndirectIllumination;
    };

    static_assert(sizeof(LightBaseInfo) == 0x68);

    struct UserColor {
        const char* mName;
        sead::Color4f mColor;
    };

    ActorPrePassLightKeeper(bool isIgnoreYaml);
    ~ActorPrePassLightKeeper();

    void init(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter);

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
    bool mIsIgnoreHideModel;
};
}  // namespace al

namespace alYamlMacroUtil {
class YamlParamGroup;
}

namespace np_LightCommon {
extern alYamlMacroUtil::YamlParamGroup LightCommon;
}
