#include "Project/Draw/GraphicsParamKeeper.hpp"

#include <attributes.h>
#include <utility/aglParameterIO.h>
#include <utility/aglResParameter.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace GraphicsParamKeeperFuncImpl {

/**
 * Checks whether a string is empty.
 * @param pStr String to check.
 * @return Whether the string is empty.
 */
bool isEmptyString(const char* pStr) {
    return al::isEqualString(pStr, "");
}

/**
 * Checks whether a string matches a wildcard pattern.
 * @param pStr String to check.
 * @param pPattern Pattern to match against.
 * @return Whether the string matches the pattern.
 */
bool isMatchString(const char* pStr, const char* pPattern) {
    return al::isMatchString(pStr, al::MatchStr(pPattern));
}

}  // namespace GraphicsParamKeeperFuncImpl

namespace al {

/**
 * Constructs a keeper for the stage graphics parameter binaries of one type.
 * @param pInfo Graphics system info.
 * @param pParamIo Parameter IO the binaries are applied to.
 * @param pName Name of the parameter type.
 * @param pExtension Extension of the parameter binaries.
 * @param paramType Graphics area parameter type.
 */
GraphicsParamKeeperImpl::GraphicsParamKeeperImpl(GraphicsSystemInfo* pInfo,
                                                 agl::utl::IParameterIO* pParamIo,
                                                 const char* pName, const char* pExtension,
                                                 s32 paramType)
    : mGraphicsSystemInfo(pInfo), mFilePath(new GraphicsParamFilePath("Default", pExtension)),
      mParamIo(pParamIo), mParamType(paramType) {
    mName = pName;
}

/**
 * Collects all parameter binaries of the stage and applies the default one.
 * @param pResource Stage resource.
 * @param pStageName Name of the stage.
 */
void GraphicsParamKeeperImpl::initStageResource(const Resource* pResource, const char* pStageName) {
    if (pResource == nullptr) {
        return;
    }

    StringTmp<32> suffix(".b%s", mFilePath->getExtension());
    u32 entryNum = pResource->getEntryNum("/");

    for (u32 i = 0; i < entryNum; i++) {
        StringTmp<256> entryName;
        pResource->getEntryName(&entryName, "/", i);

        if (!isEqualSubString(entryName.cstr(), suffix.cstr())) {
            continue;
        }

        auto* binary = new ParamBinary;
        removeExtensionString(binary->name.getBuffer(), binary->name.getBufferSize(),
                              entryName.cstr());
        binary->data = pResource->getOtherFile(entryName, nullptr);
        mParamBinaries.pushBack(binary);
    }

    const ParamBinary* binary = tryFindParamBinary("Default");

    if (binary != nullptr) {
        mParamIo->applyResParameterArchive(agl::utl::ResParameterArchive(binary->data));
        mIsLoaded = true;
    } else {
        mIsLoaded = false;
    }
}

/**
 * Finds a parameter binary by name.
 * @param pName Name of the binary.
 * @return The binary, or nullptr if not found.
 */
const GraphicsParamKeeperImpl::ParamBinary*
GraphicsParamKeeperImpl::tryFindParamBinary(const char* pName) const {
    if (pName == nullptr || isEqualString(pName, "")) {
        return nullptr;
    }

    s32 binaryNum = mParamBinaries.size();

    for (s32 i = 0; i < binaryNum; i++) {
        ParamBinary* binary = mParamBinaries.unsafeAt(i);

        if (isEqualString(pName, binary->name.cstr())) {
            return binary;
        }
    }

    return nullptr;
}

/**
 * Applies the parameters of the current graphics area.
 */
void GraphicsParamKeeperImpl::updateRequest() {
    if (!mIsLoaded) {
        return;
    }

    GraphicsAreaDirector* areaDirector = mGraphicsSystemInfo->getGraphicsAreaDirector();

    if (areaDirector == nullptr) {
        return;
    }

    CurrentGraphicsAreaParam areaParam;
    areaDirector->getCurrentGraphicsAreaParam(&areaParam,
                                              static_cast<GraphicsAreaParamType>(mParamType));
    const ParamBinary* binary = tryFindParamOrDefaultBinary(areaParam.mParamName);
    const ParamBinary* prevBinary = tryFindParamOrDefaultBinary(areaParam.mPrevParamName);

    if (binary == nullptr) {
        return;
    }

    if (!areaParam.mIsLerp && mIsApplied) {
        return;
    }

    agl::utl::IParameterIO* paramIo = mParamIo;

    if (prevBinary == nullptr) {
        paramIo->applyResParameterArchive(agl::utl::ResParameterArchive(binary->data));
    } else {
        paramIo->applyResParameterArchiveLerp(agl::utl::ResParameterArchive(prevBinary->data),
                                              agl::utl::ResParameterArchive(binary->data),
                                              areaParam.mRate);
    }

    mIsApplied = true;
}

/**
 * Finds a parameter binary by name, falling back to the default binary.
 * @param pName Name of the binary.
 * @return The binary, or nullptr if neither exists.
 */
const GraphicsParamKeeperImpl::ParamBinary*
GraphicsParamKeeperImpl::tryFindParamOrDefaultBinary(const char* pName) const {
    const ParamBinary* binary = tryFindParamBinary(pName);

    if (binary != nullptr) {
        return binary;
    }

    return tryFindParamBinary("Default");
}

/**
 * Constructs the base of a keeper for requestable and interpolated graphics parameters.
 * @param pInfo Graphics system info.
 * @param paramType Graphics area parameter type.
 * @param pDirName Directory name of the parameter files.
 * @param pExtension Extension of the parameter files.
 * @param pTypeName Type name of the parameters, the directory name if nullptr.
 */
GraphicsParamRequestInterpKeeperImpl::GraphicsParamRequestInterpKeeperImpl(
    GraphicsSystemInfo* pInfo, s32 paramType, const char* pDirName, const char* pExtension,
    const char* pTypeName)
    : mGraphicsSystemInfo(pInfo), mParamIo(new GraphicsParamIo(pDirName, pExtension, pTypeName)),
      mParamType(paramType) {}

/**
 * Loads the stage parameter file and checks which named parameters exist.
 * @param pResource Stage resource.
 * @param pStageName Name of the stage.
 */
void GraphicsParamRequestInterpKeeperImpl::initStageResource(const Resource* pResource,
                                                             const char* pStageName) {
    mParamIo->initStageResource(pResource, pStageName);
    checkNamedParamExistance();
}

/**
 * Checks whether at least one named parameter exists.
 * @return Whether a named parameter exists.
 */
bool GraphicsParamRequestInterpKeeperImpl::isExistNamedParamAtLeastOne() const {
    return mIsExistNamedParam;
}

/**
 * Gets the directory name of the parameter files.
 * @return Directory name.
 */
const char* GraphicsParamRequestInterpKeeperImpl::getParamDirName() const {
    return mParamIo->getDirName();
}

/**
 * Gets the type name of the parameters.
 * @return Type name.
 */
const char* GraphicsParamRequestInterpKeeperImpl::getParamTypeName() const {
    return mParamIo->getTypeName();
}

/**
 * Gets the parameter IO.
 * @return Parameter IO.
 */
agl::utl::IParameterIO* GraphicsParamRequestInterpKeeperImpl::getParamIo() {
    return mParamIo->getParamIo();
}

/**
 * Gets the graphics area director.
 * @return Graphics area director.
 */
GraphicsAreaDirector* GraphicsParamRequestInterpKeeperImpl::getGraphicsAreaDirector() {
    return mGraphicsSystemInfo->getGraphicsAreaDirector();
}

/**
 * Constructs the IO of a parameter file.
 * @param pDirName Directory name of the parameter file.
 * @param pExtension Extension of the parameter file.
 * @param pTypeName Type name of the parameters, the directory name if nullptr.
 */
NOINLINE GraphicsParamIo::GraphicsParamIo(const char* pDirName, const char* pExtension,
                                 const char* pTypeName)
    : mFilePath(new GraphicsParamFilePath(pDirName, pExtension)), mDirName(pDirName),
      mTypeName(pTypeName != nullptr ? pTypeName : pDirName) {}

/**
 * Applies the stage parameter file if it exists.
 * @param pResource Stage resource.
 * @param pStageName Name of the stage.
 */
void GraphicsParamIo::initStageResource(const Resource* pResource, const char* pStageName) {
    if (pResource == nullptr) {
        return;
    }

    StringTmp<256> path;
    mFilePath->makeBinaryPath(&path);

    if (pResource->isExistFile(path)) {
        const void* file = pResource->getOtherFile(path, nullptr);
        mParamIo.applyResParameterArchive(agl::utl::ResParameterArchive(file));
    }
}

/**
 * Constructs the path of a graphics parameter file.
 * @param pName Name of the file.
 * @param pExtension Extension of the file.
 */
NOINLINE
GraphicsParamFilePath::GraphicsParamFilePath(const char* pName, const char* pExtension)
    : mName(pName), mExtension(pExtension) {}

/**
 * Makes the path of the binary parameter file.
 * @param pPath Output path.
 */
NOINLINE void GraphicsParamFilePath::makeBinaryPath(StringTmp<256>* pPath) const {
    pPath->format("%s.b%s", mName.cstr(), mExtension.cstr());
}

}  // namespace al
