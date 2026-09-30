#pragma once

#include <basis/seadTypes.h>

#include "Project/Animation/AnimPlayerBase.hpp"

namespace nn::g3d {
class ModelObj;
}

namespace al {
class InitResourceDataAnim;
class Resource;

struct AnimPlayerInitInfo {
    Resource* mAnimResource;
    nn::g3d::ModelObj* mModelObj;
    Resource* mModelResource;
    InitResourceDataAnim* mInitResourceDataAnim;
};

class AnimPlayerSimple : public AnimPlayerBase {
public:
    AnimPlayerSimple();

    void update();
    void startAnim(const char* pName);
    void clearAnim();
    f32 getAnimFrame() const;
    void setAnimFrame(f32 frame);
    f32 getAnimFrameMax() const;
    f32 getAnimFrameMax(const char* pName) const;
    f32 getAnimFrameRate() const;
    void setAnimFrameRate(f32 rate);
    bool isAnimExist(const char* pName) const;
    bool isAnimEnd() const;
    bool isAnimOneTime() const;
    bool isAnimOneTime(const char* pName) const;
    bool isAnimPlaying() const;
    const char* getPlayingAnimName() const;
};

class AnimPlayerMat : public AnimPlayerSimple {
public:
    static AnimPlayerMat* tryCreate(const AnimPlayerInitInfo* pInfo, s32 type);
};

class AnimPlayerVis : public AnimPlayerSimple {
public:
    static AnimPlayerVis* tryCreate(const AnimPlayerInitInfo* pInfo);
};
}  // namespace al
