#pragma once

#include <basis/seadTypes.h>

#include "Project/Animation/AnimPlayerBase.hpp"

namespace nn::g3d {
class ModelAnimObj;
class ModelObj;
}  // namespace nn::g3d

namespace al {
class InitResourceDataAnim;
class Resource;
struct AnimResInfo;

struct AnimPlayerInitInfo {
    Resource* mAnimResource;
    nn::g3d::ModelObj* mModelObj;
    Resource* mModelResource;
    InitResourceDataAnim* mInitResourceDataAnim;
};

/**
 * Pair of a model and the animation object bound to it.
 */
struct AnimPlayerModelAnim {
    nn::g3d::ModelObj* mModelObj = nullptr;
    nn::g3d::ModelAnimObj* mAnimObj = nullptr;
};

class AnimPlayerSimple : public AnimPlayerBase {
public:
    AnimPlayerSimple();

    virtual void init(const AnimPlayerInitInfo* pInfo) = 0;
    virtual void setAnimToModel(const AnimResInfo* pInfo) = 0;
    virtual void applyTo();

    bool calcNeedUpdateAnimNext() override;

    void startAnim(const char* pName);
    void update();
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

protected:
    AnimPlayerModelAnim* mModelAnim;
    const AnimResInfo* mPlayingAnim = nullptr;
};

static_assert(sizeof(AnimPlayerSimple) == 0x28);

class AnimPlayerMat : public AnimPlayerSimple {
public:
    static AnimPlayerMat* tryCreate(const AnimPlayerInitInfo* pInfo, s32 matType);

    AnimPlayerMat(s32 matType) : mMatType(matType) {}

    void init(const AnimPlayerInitInfo* pInfo) override;
    void setAnimToModel(const AnimResInfo* pInfo) override;

private:
    s32 mMatType;
    Resource* mModelResource = nullptr;
    Resource* mAnimResource = nullptr;
};

static_assert(sizeof(AnimPlayerMat) == 0x40);

class AnimPlayerVis : public AnimPlayerSimple {
public:
    static AnimPlayerVis* tryCreate(const AnimPlayerInitInfo* pInfo);

    AnimPlayerVis();

    void init(const AnimPlayerInitInfo* pInfo) override;
    void setAnimToModel(const AnimResInfo* pInfo) override;
};

static_assert(sizeof(AnimPlayerVis) == 0x28);

}  // namespace al
