#include "Project/Base/ParamRequestInterp.hpp"

#include "Library/Yaml/ParameterBase.hpp"

namespace al {
/**
 * Checks if the parameters are equal to another request parameter's.
 * @param rOther other request parameter
 * @return whether the parameters are equal
 */
bool IUseRequestParam::isEqual(const IUseRequestParam& rOther) const {
    if (!getParamObj()) {
        return false;
    }

    return getParamObj()->isEqual(*rOther.getParamObj());
}

/**
 * Copies another request parameter's parameters.
 * @param rOther other request parameter
 */
void IUseRequestParam::copy(const IUseRequestParam& rOther) {
    if (!getParamObj()) {
        return;
    }

    getParamObj()->copy(*rOther.getParamObj());
}

/**
 * Interpolates between two request parameters' parameters.
 * @param rParamA start parameter
 * @param rParamB end parameter
 * @param rate interpolation rate
 */
void IUseRequestParam::copyInterp(const IUseRequestParam& rParamA,
                                  const IUseRequestParam& rParamB, f32 rate) {
    if (!getParamObj()) {
        return;
    }

    getParamObj()->copyLerp(*rParamA.getParamObj(), *rParamB.getParamObj(), rate);
}

/**
 * Constructs a parameter request interpolator.
 */
ParamRequestInterp::ParamRequestInterp() = default;

/**
 * Calculates the interpolation rate.
 * @return interpolation rate
 */
f32 ParamRequestInterp::calcRate() const {
    if (mStep == -1 || mStep == 0) {
        return 1.0f;
    }

    return static_cast<f32>(mStepMax - mStep) / mStepMax;
}

/**
 * Clears the request of the current frame.
 */
void ParamRequestInterp::clearRequest() {
    mIsSamePriorityRequested = false;
    mIsRequested = false;
    mPriority = -2;
}

/**
 * Ends initialization, so the next request is interpolated.
 */
void ParamRequestInterp::endInit() {
    mIsEndInit = true;
}

/**
 * Checks if a parameter was requested.
 * @return whether a parameter was requested
 */
bool ParamRequestInterp::isRequested() const {
    return mIsRequested;
}

/**
 * Requests a parameter to interpolate to.
 * @param priority request priority
 * @param step interpolation frames
 * @param rParam requested parameter
 * @return whether a request with the same priority was already made
 */
bool ParamRequestInterp::requestParam(s32 priority, s32 step, const IUseRequestParam& rParam) {
    s32 requestStep = step > 1 ? step : 1;
    if (mIsFirstRequest) {
        return requestParamDirect_(priority, rParam);
    }

    if (mPriority < priority) {
        mIsRequested = true;
        mPriority = priority;
        mRequestStep = requestStep;
        mRequestParam->copy(rParam);
        mIsSamePriorityRequested = false;
        return false;
    }

    if (mPriority == priority) {
        bool isSamePriorityRequested = true;
        if (!mIsSamePriorityRequested) {
            isSamePriorityRequested = false;
            mIsRequested = true;
        }

        mIsSamePriorityRequested = true;
        return isSamePriorityRequested;
    }

    mIsRequested = true;
    return false;
}

/**
 * Requests a parameter to apply without interpolation.
 * @param priority request priority
 * @param rParam requested parameter
 * @return whether a request with the same priority was already made
 */
bool ParamRequestInterp::requestParamDirect_(s32 priority, const IUseRequestParam& rParam) {
    if (mPriority < priority) {
        mIsSamePriorityRequested = false;
        mCurrentParam->copy(rParam);
        mStartParam->copy(rParam);
        mRequestParam->copy(rParam);
        mPriority = priority;
        mRequestStep = 1;
        mStepMax = 1;
        mIsRequested = true;
        return false;
    }

    if (mPriority == priority) {
        bool isSamePriorityRequested = true;
        if (!mIsSamePriorityRequested) {
            isSamePriorityRequested = false;
            mIsRequested = true;
        }

        mIsSamePriorityRequested = true;
        return isSamePriorityRequested;
    }

    mIsRequested = true;
    return false;
}

/**
 * Starts a new interpolation if the request changed and updates the current parameter.
 */
void ParamRequestInterp::updateInterp() {
    if (!mEndParam->isEqual(*mRequestParam)) {
        mStep = mRequestStep;
        mStepMax = mRequestStep;
        mEndParam->copy(*mRequestParam);
        mStartParam->copy(*mCurrentParam);
    }

    s32 step = mStep - 1;
    mStep = step >= 0 ? step : -1;
    if (mStep < 0) {
        mPriority = -2;
        return;
    }

    f32 rate = calcRate();
    mCurrentParam->copyInterp(*mStartParam, *mEndParam, rate);
    if (mIsEndInit && mIsFirstRequest) {
        mIsFirstRequest = false;
    }
}
}  // namespace al
