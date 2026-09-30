#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace alYamlMacroUtil {
class YamlParamGroup;
}

namespace al {
class ActorInitInfo;
class ByamlIter;
class LiveActor;
class ShadowMaskBase;
class ShadowMaskCastOvalCylinder;
class ShadowMaskCube;
class ShadowMaskCylinder;
class ShadowMaskSphere;

class ShadowKeeper {
public:
    struct ShadowMaskBaseInfo {
        void setPtr();
        void readIter(const ByamlIter& rIter);

        const char* mName = nullptr;
        const char* mShadowMaskType = nullptr;
        const char* mActorJointName = nullptr;
        sead::Vector3f mOffset = {0.0f, 0.0f, 0.0f};
        sead::Color4f mColor = {0.2f, 0.2f, 0.2f, 1.0f};
        bool mIsApplyShadowIntensityUser = false;
        u8 mShadowIntensityUser = 0;
        bool mIsIgnoreHide = false;
        bool mIsFollowHostScale = true;
        const char* mDrawCategory = nullptr;
        bool mIsShadowFixed = false;
        const char* mSetHeightEvenTargetName = nullptr;
    };

    ShadowKeeper(bool isIgnoreShadowMaskYaml);
    ~ShadowKeeper();

    void initShadowMaskNum(s32 maskNum);
    bool init(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter);
    ShadowMaskBase* findShadowMask(const char* pName) const;
    void show();
    void initAfterPlacement();
    void hide();
    void validate();
    void invalidate();
    bool isHide();

    void setupShadowMaskSphereParam(ShadowMaskSphere* pMask) const;
    void setupShadowMaskCylinderParam(ShadowMaskCylinder* pMask) const;
    void setupShadowMaskCubeParam(ShadowMaskCube* pMask) const;
    void setupShadowMaskCastOvalCylinderParam(ShadowMaskCastOvalCylinder* pMask) const;

    s32 getShadowMaskNum() const { return mMaskArray.size(); }

    ShadowMaskBase* getShadowMask(s32 index) const { return mMaskArray[index]; }

    sead::PtrArray<ShadowMaskBase> mMaskArray;
    void* _10;
    void* _18;
    void* _20;
    void* _28;
    void* _30;
    void* _38;
    LiveActor* mHostActor;
    bool mIsIgnoreShadowMaskYaml;
};

bool isShadowIntensity(s32 category);
bool isShadowLightScale(s32 category);
bool isShadowMrt(s32 category);

}  // namespace al

namespace np_ShadowMaskCommon {
extern alYamlMacroUtil::YamlParamGroup ShadowMaskCommon;
}

namespace np_ShadowMaskSphereParam {
extern alYamlMacroUtil::YamlParamGroup ShadowMaskSphereParam;
}

namespace np_ShadowMaskCylinderParam {
extern alYamlMacroUtil::YamlParamGroup ShadowMaskCylinderParam;
}

namespace np_ShadowMaskCubeParam {
extern alYamlMacroUtil::YamlParamGroup ShadowMaskCubeParam;
}

namespace np_ShadowMaskCastOvalCylinderParam {
extern alYamlMacroUtil::YamlParamGroup ShadowMaskCastOvalCylinderParam;
}
