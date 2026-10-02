#include "Project/Draw/GraphicsParamKeeper.hpp"

#include <math/seadMathCalcCommon.h>
#include <utility/aglResParameter.h>

namespace al {

/**
 * Constructs a requester for a parameter IO.
 * @param pParamIo Parameter IO the requested parameters are applied to.
 * @param pName Name of the requester (unused).
 */
GraphicsParamRequesterImpl::GraphicsParamRequesterImpl(agl::utl::IParameterIO* pParamIo,
                                                       const char* pName)
    : mParamIo(pParamIo) {}

/**
 * Marks the end of initialization.
 */
void GraphicsParamRequesterImpl::endInit() {
    mIsEndInit = true;
}

/**
 * Clears the request of the current frame.
 */
void GraphicsParamRequesterImpl::clearRequest() {
    mIsSamePriorityRequested = false;
    mPriority = -2;
}

/**
 * Takes over the latest request and applies the interpolated parameters.
 */
void GraphicsParamRequesterImpl::updateRequest() {
    if (mIsRequested) {
        mStep = mRequestStep;
        mStepNum = mRequestStep;
        mPrevData = mCurrentData;
        mCurrentData = mRequestData;
        mIsRequested = false;
    }

    mStep = sead::Mathi::max(mStep - 1, -1);

    if (mStep < 0) {
        mPriority = -2;
        return;
    }

    f32 rate = calcRate();

    if (mCurrentData != nullptr) {
        if (mPrevData != nullptr) {
            mParamIo->applyResParameterArchiveLerp(agl::utl::ResParameterArchive(mPrevData),
                                                   agl::utl::ResParameterArchive(mCurrentData),
                                                   rate);
        } else {
            mParamIo->applyResParameterArchive(agl::utl::ResParameterArchive(mCurrentData));
        }
    }

    if (mIsEndInit && mIsBeforeFirstUpdate) {
        mIsBeforeFirstUpdate = false;
    }
}

/**
 * Calculates the interpolation rate towards the current parameters.
 * @return Interpolation rate in [0, 1].
 */
f32 GraphicsParamRequesterImpl::calcRate() const {
    if (mStep == -1 || mStep == 0) {
        return 1.0f;
    }

    return static_cast<f32>(mStepNum - mStep) / mStepNum;
}

/**
 * Requests parameters to be interpolated to.
 * @param priority Priority of the request; only the highest one is used.
 * @param step Number of frames to interpolate over.
 * @param pData Parameter binary data.
 * @return Whether a request with the same priority was already made.
 */
bool GraphicsParamRequesterImpl::requestParam(s32 priority, s32 step, void* pData) {
    s32 requestStep = sead::Mathi::max(step, 1);

    if (mIsBeforeFirstUpdate) {
        return requestParamDirect(priority, pData);
    }

    if (mPriority < priority) {
        mRequestStep = requestStep;
        mPriority = priority;
        mRequestData = pData;
        mIsSamePriorityRequested = false;
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
 * Requests parameters to be applied immediately.
 * @param priority Priority of the request; only the highest one is used.
 * @param pData Parameter binary data.
 * @return Whether a request with the same priority was already made.
 */
bool GraphicsParamRequesterImpl::requestParamDirect(s32 priority, void* pData) {
    if (mPriority < priority) {
        mIsSamePriorityRequested = false;
        mIsRequested = true;
        mPrevData = pData;
        mRequestData = pData;
        mPriority = priority;
        mRequestStep = 1;
        mStepNum = 1;

        if (pData != nullptr) {
            mParamIo->applyResParameterArchive(agl::utl::ResParameterArchive(pData));
        }

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
 * Checks whether parameters were requested since the last update.
 * @return Whether parameters were requested.
 */
bool GraphicsParamRequesterImpl::isRequested() const {
    return mIsRequested;
}

}  // namespace al
