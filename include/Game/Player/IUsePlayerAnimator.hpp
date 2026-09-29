#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

/// The player's skeletal, sub and material animations (implemented by PlayerAnimator).
class IUsePlayerAnimator {
public:
    virtual ~IUsePlayerAnimator() = default;
    virtual void enableAlphaCtrl(bool) = 0;
    virtual void startAnim(const sead::SafeString&) = 0;
    virtual void setAnimRate(f32) = 0;
    virtual void setSubAnimRate(f32) = 0;
    virtual void setAnimFrame(f32) = 0;
    virtual bool isAnimEnd() const = 0;
    virtual bool isAnim(const sead::SafeString&) const = 0;
    virtual bool isAnimReverse() const = 0;
    virtual f32 getAnimFrame() const = 0;
    virtual f32 getAnimFrameMax() const = 0;
    virtual void clearInterpolation() = 0;
    virtual const char* getAnimName() const = 0;
    virtual void startSubAnim(const sead::SafeString&) = 0;
    virtual void endSubAnim() = 0;
    virtual bool isSubAnimBinding() const = 0;
    virtual bool isSubAnimEnd() const = 0;
    virtual bool isSubAnim(const sead::SafeString&) const = 0;
    virtual bool isSubAnimReverse() const = 0;
    virtual f32 getSubAnimFrame() const = 0;
    virtual f32 getAnimFrameMax(const sead::SafeString&) const = 0;
    virtual void startMaterialAnim(const sead::SafeString&) = 0;
    virtual bool isMaterialAnimEnd() const = 0;
    virtual void setWeightSixfold(f32, f32, f32, f32, f32, f32) = 0;
    virtual f32 getWeight(u32) const = 0;
    virtual bool isUpperBodyAnimAttached() const = 0;
};
