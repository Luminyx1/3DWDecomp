#pragma once

#include <basis/seadTypes.h>

#include "Project/Animation/AnimPlayerBase.hpp"

namespace al {
struct AnimPlayerInitInfo;
class SklAnimRetargettingInfo;

class AnimPlayerSkl : public AnimPlayerBase {
public:
    static AnimPlayerSkl* tryCreate(const AnimPlayerInitInfo* pInfo, s32 blendNum);

    void calcSklAnim();
    void initInterp(const char* pName);
    void update();
    void reset();
    void startSklAnim(const char* pName0, const char* pName1, const char* pName2,
                      const char* pName3, const char* pName4, const char* pName5,
                      const char* pName6);
    void setSklAnimBlendWeight(s32 index, f32 weight);
    f32 getSklAnimBlendWeight(s32 index) const;
    s32 getSklAnimBlendNum() const;
    f32 getSklAnimFrame(s32 index) const;
    void setSklAnimFrame(s32 index, f32 frame, bool isUpdate);
    f32 getSklAnimFrameMax(s32 index) const;
    f32 getSklAnimFrameMax(const char* pName) const;
    f32 getSklAnimFrameRate(s32 index) const;
    void setSklAnimFrameRate(s32 index, f32 rate);
    bool isSklAnimExist(const char* pName) const;
    bool isSklAnimEnd(s32 index) const;
    bool isSklAnimOneTime(s32 index) const;
    bool isSklAnimOneTime(const char* pName) const;
    bool isSklAnimPlaying(s32 index) const;
    const char* getPlayingSklAnimName(s32 index) const;
    void initPartialAnim(s32 slotNum, s32 jointNum, s32 partsNum);
    void addPartialAnimJoint(s32 index, const char* pJointName, const char* pEndJointName);
    void addPartialAnimJointRecursive(s32 index, const char* pJointName);
    void startPartialAnim(const char* pName, s32 index, s32 interpole,
                          const SklAnimRetargettingInfo* pInfo);
    void clearPartialAnim(s32 index);
    bool isPartialAnimEnd(s32 index) const;
    bool isPartialAnimAttached(s32 index) const;
    f32 getPartialAnimFrame(s32 index) const;
    void setPartialAnimFrame(s32 index, f32 frame);
    f32 getPartialAnimFrameRate(s32 index) const;
    void setPartialAnimFrameRate(s32 index, f32 rate);

    u8 _18[0x80 - 0x18];
    const SklAnimRetargettingInfo* mRetargettingInfo;
    bool mIsRetargettingValid;
};
}  // namespace al
