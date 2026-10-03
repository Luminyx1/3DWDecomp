#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

#include <attributes.h>
#include <common/aglShaderProgramArchive.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <filedevice/seadFileDevice.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <gfx/seadGraphics.h>
#include <heap/seadHeapMgr.h>
#include <nn/g3d/g3d_ResShader.h>

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/ForwardRendering/SkyboxDirector.hpp"
#include "Project/Base/StringUtil.hpp"

/**
 * Looks up a shading model by name.
 * @param pName Name of the shading model.
 * @return The shading model, or nullptr if it does not exist.
 */
NOINLINE inline nn::g3d::ResShadingModel*
nn::g3d::ResShaderArchive::FindShadingModel(const char* pName) {
    const nn::util::ResDic* dictionary = shadingModelDic;

    if (dictionary == nullptr) {
        return nullptr;
    }

    int index = dictionary->FindIndex(pName);

    if (index == nn::util::ResDic::Npos) {
        return nullptr;
    }

    return &models[index];
}

namespace ShaderSearchImpl {
/**
 * Compares two strings.
 * @param pA First string.
 * @param pB Second string.
 * @return Whether the strings are equal.
 */
bool isEqualStr(const char* pA, const char* pB) {
    return al::isEqualString(pA, pB);
}
}  // namespace ShaderSearchImpl

namespace al {

SEAD_SINGLETON_DISPOSER_IMPL(ShaderHolder)

/**
 * Constructs an empty shader holder.
 */
ShaderHolder::ShaderHolder() = default;

/**
 * Resets the archive tables and marks the holder as initialized.
 */
void ShaderHolder::init() {
    mIsInitialized = true;
    mProgramArchiveNum = 0;
    mShaderArchiveNum = 0;
    mUberShaderArchiveNum = 0;

    for (s32 i = 0; i < cArchiveNumMax; i++) {
        mProgramArchives[i] = nullptr;
        mShaderArchives[i] = nullptr;
    }
}

/**
 * Initializes the holder and loads every shader of an archive.
 * @param pArchiveName Path of the archive containing the shaders.
 * @param pHeap Heap used for the shader allocations.
 */
void ShaderHolder::initAndLoadAll(const char* pArchiveName, sead::Heap* pHeap) {
    init();
    loadAll(pArchiveName, pHeap);
}

/**
 * Loads every shader program archive and shader archive contained in an archive.
 * @param pArchiveName Path of the archive containing the shaders.
 * @param pHeap Heap used for the shader allocations.
 */
void ShaderHolder::loadAll(const char* pArchiveName, sead::Heap* pHeap) {
    sead::ScopedCurrentHeapSetter heapSetter(pHeap);
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    s32 entryNum = resource->getEntryNum("/");

    for (s32 i = 0; i < entryNum; i++) {
        StringTmp<256> entryName;
        resource->getEntryName(&entryName, "/", i);

        if (isEqualSubString(entryName.cstr(), ".sharcb")) {
            entryName.removeSuffix(".sharcb");
            load(pArchiveName, entryName.cstr(), pHeap, 0);
        } else if (isEqualSubString(entryName.cstr(), "Uber.bfsha")) {
            void* file = resource->getOtherFile(entryName.cstr(), nullptr);
            nn::g3d::ResShaderFile* shaderFile = nn::g3d::ResShaderFile::ResCast(file);
            mUberShaderArchives[mUberShaderArchiveNum] = shaderFile->archive;
            mUberShaderArchiveNum++;
        } else if (isEqualSubString(entryName.cstr(), ".bfsha")) {
            void* file = resource->getOtherFile(entryName.cstr(), nullptr);
            nn::g3d::ResShaderFile* shaderFile = nn::g3d::ResShaderFile::ResCast(file);
            mShaderArchives[mShaderArchiveNum] = shaderFile->archive;
            mShaderArchiveNum++;
        }
    }
}

/**
 * Initializes the holder and loads every shader archive found in a directory.
 * @param pDirName Directory containing the shader archives.
 * @param pHeap Heap used for the shader allocations.
 */
void ShaderHolder::initAndLoadAllFromDir(const char* pDirName, sead::Heap* pHeap) {
    init();
    sead::FileDevice* device = sead::FileDeviceMgr::instance()->findDevice("main");
    sead::DirectoryHandle handle;
    device = device->openDirectory(&handle, pDirName);
    sead::DirectoryEntry entries[39];
    s32 entryNum = device->readDirectory(&handle, entries, 39);

    for (s32 i = 0; i < entryNum; i++) {
        sead::FixedSafeString<256>& name = entries[i].name;
        s32 length = name.calcLength();

        if (length != name.removeSuffix(".szs")) {
            StringTmp<256> archiveName("%s/%s", pDirName, name.cstr());
            loadAll(archiveName.cstr(), pHeap);
        }
    }

    device->closeDirectory(&handle);
}

/**
 * Loads a shader program archive from its binary and source files.
 * @param pArchiveName Path of the archive containing the shader files.
 * @param pName Base name of the shader files.
 * @param pHeap Heap used for the shader allocations.
 * @param option Option flags passed to the shader program archive.
 */
void ShaderHolder::load(const char* pArchiveName, const char* pName, sead::Heap* pHeap,
                        u32 option) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    void* binaryFile = resource->getOtherFile(StringTmp<256>("%s.sharcb", pName), nullptr);
    void* sourceFile = resource->getOtherFile(StringTmp<256>("%s.sharc", pName), nullptr);

    sead::Graphics::instance()->lockDrawContext();
    mProgramArchives[mProgramArchiveNum] = new agl::ShaderProgramArchive();
    mProgramArchives[mProgramArchiveNum]->createWithOption(binaryFile, sourceFile, option, pHeap);
    mProgramArchives[mProgramArchiveNum]->setUp();
    sead::Graphics::instance()->unlockDrawContext();
    mProgramArchiveNum++;
}

/**
 * Searches all loaded shader program archives for a shader program.
 * @param pName Name of the shader program.
 * @return The shader program, or nullptr if it was not found.
 */
agl::ShaderProgram* ShaderHolder::tryGetShaderProgram(const char* pName) const {
    for (s32 i = 0; i < mProgramArchiveNum; i++) {
        agl::ShaderProgramArchive* archive = mProgramArchives[i];

        if (archive == nullptr) {
            continue;
        }

        s32 index = archive->searchShaderProgramIndex(pName);

        if (index < 0) {
            continue;
        }

        agl::ShaderProgram* program = archive->getShaderProgramPtr(index);

        if (program != nullptr) {
            return program;
        }
    }

    return nullptr;
}

/**
 * Gets a shader program from the loaded shader program archives.
 * @param pName Name of the shader program.
 * @return The shader program, or nullptr if it was not found.
 */
agl::ShaderProgram* ShaderHolder::getShaderProgram(const char* pName) const {
    return tryGetShaderProgram(pName);
}

/**
 * Searches all loaded shader archives for a shading model.
 * @param pName Name of the shading model.
 * @return The shading model, or nullptr if it was not found.
 */
nn::g3d::ResShadingModel* ShaderHolder::getShadingModel(const char* pName) const {
    for (s32 i = 0; i < mShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mShaderArchives[i];

        if (archive == nullptr) {
            continue;
        }

        nn::g3d::ResShadingModel* shadingModel = archive->FindShadingModel(pName);

        if (shadingModel != nullptr) {
            return shadingModel;
        }
    }

    return nullptr;
}

/**
 * Searches all loaded uber shader archives for a shading model.
 * @param pName Name of the shading model.
 * @return The shading model, or nullptr if it was not found.
 */
nn::g3d::ResShadingModel* ShaderHolder::getShadingModelUber(const char* pName) const {
    for (s32 i = 0; i < mUberShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mUberShaderArchives[i];

        if (archive == nullptr) {
            continue;
        }

        nn::g3d::ResShadingModel* shadingModel = archive->FindShadingModel(pName);

        if (shadingModel != nullptr) {
            return shadingModel;
        }
    }

    return nullptr;
}

/**
 * Sets up every loaded shader archive and uber shader archive on the graphics device.
 */
void ShaderHolder::setupShaderArchives() {
    for (s32 i = 0; i < mShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mShaderArchives[i];

        if (archive != nullptr) {
            archive->Setup(static_cast<nn::gfx::Device*>(
                               agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()),
                           nullptr, 0);
        }
    }

    for (s32 i = 0; i < mUberShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mUberShaderArchives[i];

        if (archive != nullptr) {
            archive->Setup(static_cast<nn::gfx::Device*>(
                               agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()),
                           nullptr, 0);
        }
    }
}

/**
 * Cleans up every loaded shader archive and uber shader archive.
 */
void ShaderHolder::cleanupShaderArchives() {
    for (s32 i = 0; i < mShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mShaderArchives[i];

        if (archive != nullptr) {
            archive->Cleanup(static_cast<nn::gfx::Device*>(
                agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()));
        }
    }

    for (s32 i = 0; i < mUberShaderArchiveNum; i++) {
        nn::g3d::ResShaderArchive* archive = mUberShaderArchives[i];

        if (archive != nullptr) {
            archive->Cleanup(static_cast<nn::gfx::Device*>(
                agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()));
        }
    }
}

/**
 * Constructs the skybox parameters with their default values.
 */
SkyboxParam::SkyboxParam() {
    mModelName.init(sead::FixedSafeString<64>("No Name"), "skybox_name", "Skybox Name",
                    &mParamObj);
    mTextureName.init(sead::FixedSafeString<64>("No Name"), "skybox_animation_name",
                      "Skybox Animation Name", &mParamObj);
    mRotateDegreeY.init(0.0f, "vertical_offset", "Vertical Offset", "Min=-10000,Max=10000",
                        &mParamObj);
}

/**
 * Constructs named skybox parameters with an empty name.
 */
NamedSkyboxParam::NamedSkyboxParam() {
    mName.init(sead::FixedSafeString<64>(""), "Name", "Name", &mParamObj);
}

/**
 * Compares two skybox parameters.
 * @param rOther Parameters to compare against.
 * @return Always false.
 */
bool SkyboxParam::operator==(const SkyboxParam& rOther) const {
    return false;
}

/**
 * Assigns skybox parameters. Does nothing.
 * @param rOther Parameters to copy.
 * @return This object.
 */
SkyboxParam& SkyboxParam::operator=(const SkyboxParam& rOther) {
    return *this;
}

/**
 * Interpolates between two skybox parameters. Does nothing.
 * @param rA First parameters.
 * @param rB Second parameters.
 * @param rate Interpolation rate.
 */
void SkyboxParam::interp(const SkyboxParam& rA, const SkyboxParam& rB, f32 rate) {}

}  // namespace al
