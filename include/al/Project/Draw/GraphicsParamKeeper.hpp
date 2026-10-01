#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterIO.h>

#include "Project/Base/RequestInterp.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"

namespace GraphicsParamKeeperFuncImpl {
bool isEmptyString(const char* pStr);
bool isMatchString(const char* pStr, const char* pPattern);
}  // namespace GraphicsParamKeeperFuncImpl

namespace al {
class GraphicsSystemInfo;
class Resource;

class GraphicsParamFilePath {
public:
    GraphicsParamFilePath(const char* pName, const char* pExtension);

    void makeBinaryPath(StringTmp<256>* pPath) const;

    const char* getExtension() const { return mExtension.cstr(); }

private:
    sead::FixedSafeString<256> mName;
    sead::FixedSafeString<256> mExtension;
};

static_assert(sizeof(GraphicsParamFilePath) == 0x230);

class GraphicsParamIo {
public:
    GraphicsParamIo(const char* pDirName, const char* pExtension, const char* pTypeName);

    void initStageResource(const Resource* pResource, const char* pStageName);

    agl::utl::IParameterIO* getParamIo() { return &mParamIo; }

    const char* getDirName() const { return mDirName; }

    const char* getTypeName() const { return mTypeName; }

private:
    GraphicsParamFilePath* mFilePath;
    agl::utl::IParameterIO mParamIo;
    const char* mDirName;
    const char* mTypeName;
};

static_assert(sizeof(GraphicsParamIo) == 0x238);

class GraphicsParamKeeperImpl {
public:
    struct ParamBinary {
        sead::FixedSafeString<256> name;
        const void* data = nullptr;
    };

    GraphicsParamKeeperImpl(GraphicsSystemInfo* pInfo, agl::utl::IParameterIO* pParamIo,
                            const char* pName, const char* pExtension, s32 paramType);

    void initStageResource(const Resource* pResource, const char* pStageName);
    const ParamBinary* tryFindParamBinary(const char* pName) const;
    void updateRequest();
    const ParamBinary* tryFindParamOrDefaultBinary(const char* pName) const;

private:
    sead::FixedPtrArray<ParamBinary, 32> mParamBinaries;
    GraphicsSystemInfo* mGraphicsSystemInfo;
    GraphicsParamFilePath* mFilePath;
    agl::utl::IParameterIO* mParamIo;
    const char* mName;
    bool mIsLoaded = false;
    u32 mParamType;
    bool mIsApplied = false;
};

static_assert(sizeof(GraphicsParamKeeperImpl) == 0x140);

class GraphicsParamRequestInterpKeeperImpl {
public:
    GraphicsParamRequestInterpKeeperImpl(GraphicsSystemInfo* pInfo, s32 paramType,
                                         const char* pDirName, const char* pExtension,
                                         const char* pTypeName);

    virtual void initStageResource(const Resource* pResource, const char* pStageName);
    virtual void checkNamedParamExistance() = 0;

    bool isExistNamedParamAtLeastOne() const;
    const char* getParamDirName() const;
    const char* getParamTypeName() const;
    agl::utl::IParameterIO* getParamIo();
    GraphicsAreaDirector* getGraphicsAreaDirector();

protected:
    GraphicsSystemInfo* mGraphicsSystemInfo;
    GraphicsParamIo* mParamIo;
    u32 mParamType;
    bool mIsExistNamedParam = false;
};

static_assert(sizeof(GraphicsParamRequestInterpKeeperImpl) == 0x20);

/**
 * Keeps a default parameter plus up to 64 named parameters that graphics areas can request by
 * name, and interpolates the current parameter towards the requested one.
 */
template <typename T>
class GraphicsParamRequestInterpKeeper : public GraphicsParamRequestInterpKeeperImpl {
public:
    class NamedParam : public T {
    public:
        NamedParam() {
            mName.init(sead::FixedSafeString<64>("No Name"), "Name", "名前", T::getParamObj());
            T::init();
        }

        const char* getName() const { return mName->cstr(); }

        agl::utl::Parameter<sead::FixedSafeString<64>> mName;
    };

    GraphicsParamRequestInterpKeeper(GraphicsSystemInfo* pInfo, s32 paramType,
                                     const char* pDirName, const char* pExtension,
                                     const char* pTypeName)
        : GraphicsParamRequestInterpKeeperImpl(pInfo, paramType, pDirName, pExtension,
                                               pTypeName) {
        mDefaultParam.init();
        mRequestInterp.init();
        getParamIo()->addObj(mDefaultParam.getParamObj(),
                             StringTmp<128>("Default%s", getParamTypeName()).cstr());

        mNamedParamNames.tryAllocBuffer(mNamedParams.capacity(), nullptr);

        for (s32 i = 0; i < mNamedParams.capacity(); i++) {
            NamedParam* namedParam = new NamedParam();
            namedParam->mName->format("%s%02d", getParamTypeName(), i);
            mNamedParams.pushBack(namedParam);
            mNamedParamNames[i] = *namedParam->mName;
            getParamIo()->addObj(namedParam->getParamObj(), mNamedParamNames[i]);
        }
    }

    void checkNamedParamExistance() override {
        for (s32 i = 0; i < mNamedParams.size(); i++) {
            NamedParam* namedParam = mNamedParams[i];
            const char* name = namedParam->mName->cstr();

            if (!GraphicsParamKeeperFuncImpl::isMatchString(
                    name, StringTmp<64>("%s*", getParamTypeName()).cstr()) &&
                !namedParam->mName->isEmpty()) {
                mIsExistNamedParam = true;
                return;
            }
        }

        mIsExistNamedParam = false;
    }

    NamedParam* tryFindNamedParam(const char* pName) {
        s32 namedParamNum = mNamedParams.size();

        for (s32 i = 0; i < namedParamNum; i++) {
            NamedParam* namedParam = mNamedParams[i];

            if (isEqualString(pName, namedParam->mName->cstr()))
                return namedParam;
        }

        return nullptr;
    }

    void updateRequest() {
        if (!isExistNamedParamAtLeastOne() && !mIsForceUpdate)
            return;

        GraphicsAreaDirector* areaDirector = getGraphicsAreaDirector();

        if (!areaDirector) {
            mRequestInterp.requestParam(-1, 1, mDefaultParam);
        } else {
            CurrentGraphicsAreaParam areaParam;
            areaDirector->getCurrentGraphicsAreaParam(
                &areaParam, static_cast<GraphicsAreaParamType>(mParamType));
            const char* paramName = areaParam.mParamName;
            NamedParam* namedParam = nullptr;

            if (paramName && !GraphicsParamKeeperFuncImpl::isEmptyString(paramName))
                namedParam = tryFindNamedParam(paramName);
            if (namedParam)
                mRequestInterp.requestParam(areaParam.mPriority, areaParam._14, *namedParam);
            else
                mRequestInterp.requestParam(-1, 1, mDefaultParam);
        }

        mRequestInterp.updateInterp();
    }

    const T& getCurrentParam() const { return mRequestInterp.getCurrentParam(); }

    T& getCurrentParam() { return mRequestInterp.getCurrentParam(); }

    const T& getDefaultParam() const { return mDefaultParam; }

    RequestInterp<T>& getRequestInterp() { return mRequestInterp; }

    void setForceUpdate(bool isForceUpdate) { mIsForceUpdate = isForceUpdate; }

protected:
    sead::FixedPtrArray<NamedParam, 64> mNamedParams;
    RequestInterp<T> mRequestInterp;
    T mDefaultParam;
    bool mIsForceUpdate = false;
    sead::Buffer<sead::FixedSafeString<64>> mNamedParamNames;
};

/**
 * Interpolates a parameter IO towards the highest-priority request made each frame.
 */
class GraphicsParamRequesterImpl {
public:
    GraphicsParamRequesterImpl(agl::utl::IParameterIO* pParamIo, const char* pName);

    void endInit();
    void clearRequest();
    void updateRequest();
    f32 calcRate() const;
    void requestParam(s32 priority, s32 step, void* pData);
    void requestParamDirect(s32 priority, void* pData);
    bool isRequested() const;

private:
    agl::utl::IParameterIO* mParamIo;
    u8 _8[0x30];
};

static_assert(sizeof(GraphicsParamRequesterImpl) == 0x38);

/**
 * Parameter requester that owns the parameter IO it interpolates.
 */
template <typename T>
class GraphicsParamRequester : public GraphicsParamRequesterImpl {
public:
    GraphicsParamRequester(T* pParam, const char* pName)
        : GraphicsParamRequesterImpl(pParam, pName), mParam(pParam) {}

    ~GraphicsParamRequester() { delete mParam; }

    T* getParam() const { return mParam; }

private:
    T* mParam;
};

}  // namespace al
