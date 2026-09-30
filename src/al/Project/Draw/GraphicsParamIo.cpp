#include "Project/Draw/GraphicsParamKeeper.hpp"

#include <utility/aglResParameter.h>

#include "Library/Resource/Resource.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Constructs the IO of a parameter file.
 * @param pDirName Directory name of the parameter file.
 * @param pExtension Extension of the parameter file.
 * @param pTypeName Type name of the parameters, the directory name if nullptr.
 */
GraphicsParamIo::GraphicsParamIo(const char* pDirName, const char* pExtension,
                                 const char* pTypeName)
    : mFilePath(new GraphicsParamFilePath(pDirName, pExtension)), mDirName(pDirName),
      mTypeName(pTypeName ? pTypeName : pDirName) {}

/**
 * Applies the stage parameter file if it exists.
 * @param pResource Stage resource.
 * @param pStageName Name of the stage.
 */
void GraphicsParamIo::initStageResource(const Resource* pResource, const char* pStageName) {
    if (!pResource) {
        return;
    }

    StringTmp<256> path;
    mFilePath->makeBinaryPath(&path);

    if (pResource->isExistFile(path)) {
        mParamIo.applyResParameterArchive(
            agl::utl::ResParameterArchive(pResource->getOtherFile(path, nullptr)));
    }
}

/**
 * Constructs the path of a graphics parameter file.
 * @param pName Name of the file.
 * @param pExtension Extension of the file.
 */
GraphicsParamFilePath::GraphicsParamFilePath(const char* pName, const char* pExtension)
    : mName(pName), mExtension(pExtension) {}

/**
 * Makes the path of the binary parameter file.
 * @param pPath Output path.
 */
void GraphicsParamFilePath::makeBinaryPath(StringTmp<256>* pPath) const {
    pPath->format("%s.b%s", mName.cstr(), mExtension.cstr());
}

}  // namespace al
