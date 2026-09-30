#pragma once

#include <basis/seadTypes.h>

class RequestInterpMathImpl {
public:
    static s32 max(s32, s32);
};

namespace al {

/**
 * Interpolates a parameter object towards the highest-priority request made each frame.
 * T needs init(), operator==, operator= and interp(a, b, rate).
 */
template <typename T>
class RequestInterp {
public:
    RequestInterp() : mCurrentParam() {}

    void init() {
        mCurrentParam.init();
        mFromParam.init();
        mToParam.init();
        mRequestParam.init();
    }

    void requestParam(s32 priority, s32 step, const T& rParam) {
        s32 interpStep = RequestInterpMathImpl::max(1, step);
        if (mIsDirect) {
            requestParamDirect_(priority, rParam);
            return;
        }

        if (mPriority < priority) {
            mIsRequested = true;
            mPriority = priority;
            mRequestStep = interpStep;
            mRequestParam = rParam;
            mIsSamePriorityRequested = false;
        } else if (mPriority == priority) {
            if (!mIsSamePriorityRequested)
                mIsRequested = true;
            mIsSamePriorityRequested = true;
        } else {
            mIsRequested = true;
        }
    }

    bool requestParamDirect_(s32 priority, const T& rParam) {
        if (mPriority < priority) {
            mIsSamePriorityRequested = false;
            mCurrentParam = rParam;
            mFromParam = rParam;
            mRequestParam = rParam;
            mPriority = priority;
            mRequestStep = 1;
            mStepMax = 1;
            mIsRequested = true;
            return false;
        }

        bool isSamePriorityRequested = false;
        if (mPriority == priority) {
            if (mIsSamePriorityRequested)
                isSamePriorityRequested = true;
            else
                mIsRequested = true;
            mIsSamePriorityRequested = true;
        } else {
            mIsRequested = true;
        }

        return isSamePriorityRequested;
    }

    void updateInterp() {
        if (!(mToParam == mRequestParam)) {
            mStep = mRequestStep;
            mToParam = mRequestParam;
            mFromParam = mCurrentParam;
            mStepMax = mRequestStep;
        }

        mStep--;
        mStep = RequestInterpMathImpl::max(-1, mStep);
        if (mStep < 0) {
            mPriority = -2;
            if (mStep == -1)
                return;
        }

        f32 rate = mStep == 0 ? 1.0f : (f32)(mStepMax - mStep) / (f32)mStepMax;
        mCurrentParam.interp(mFromParam, mToParam, rate);
        if (mIsEndInit && mIsDirect)
            mIsDirect = false;
    }

    void endInit() { mIsEndInit = true; }

    void clearRequest() {
        mIsSamePriorityRequested = false;
        mIsRequested = false;
        mPriority = -2;
    }

    const T& getCurrentParam() const { return mCurrentParam; }

    T& getCurrentParam() { return mCurrentParam; }

    bool isRequested() const { return mIsRequested; }

private:
    s32 mPriority = -2;
    s32 mStep = -1;
    s32 mStepMax = 1;
    bool mIsSamePriorityRequested = false;
    bool mIsRequested = false;
    bool mIsEndInit = false;
    bool mIsDirect = true;
    T mCurrentParam;
    T mFromParam;
    T mToParam;
    s32 mRequestStep = -1;
    T mRequestParam;
};

}  // namespace al
