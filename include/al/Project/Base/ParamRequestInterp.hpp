#pragma once

#include <basis/seadTypes.h>

namespace al {
class ParameterObj;

class IUseRequestParam {
public:
    virtual const char* getParamName() const = 0;
    virtual ParameterObj* getParamObj() = 0;
    virtual const ParameterObj* getParamObj() const = 0;
    virtual bool isEqual(const IUseRequestParam& rOther) const;
    virtual void copy(const IUseRequestParam& rOther);
    virtual void copyInterp(const IUseRequestParam& rParamA, const IUseRequestParam& rParamB,
                            f32 rate);
};

class ParamRequestInterp {
public:
    ParamRequestInterp();

    f32 calcRate() const;
    void clearRequest();
    void endInit();
    bool isRequested() const;
    bool requestParam(s32 priority, s32 step, const IUseRequestParam& rParam);
    bool requestParamDirect_(s32 priority, const IUseRequestParam& rParam);
    void updateInterp();

    const IUseRequestParam* getCurrentParam() const { return mCurrentParam; }

    IUseRequestParam* getParam(s32 index) const { return (&mCurrentParam)[index]; }

    s32 mPriority = -2;
    s32 mStep = -1;
    s32 mStepMax = 1;
    bool mIsSamePriorityRequested = false;
    bool mIsRequested = false;
    bool mIsEndInit = false;
    bool mIsFirstRequest = true;
    IUseRequestParam* mCurrentParam = nullptr;
    IUseRequestParam* mStartParam = nullptr;
    IUseRequestParam* mEndParam = nullptr;
    s32 mRequestStep = -1;
    IUseRequestParam* mRequestParam = nullptr;
};

static_assert(sizeof(ParamRequestInterp) == 0x38);

}  // namespace al
