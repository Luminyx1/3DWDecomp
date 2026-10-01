#include "Library/Resource/Resource.hpp"

#include <filedevice/seadArchiveFileDevice.h>
#include <heap/seadHeapMgr.h>
#include <nn/g3d/g3d_ResFile.h>
#include <g3d/aglNW4FToNN.h>
#include <resource/seadArchiveRes.h>

#include "Library/File/FileUtil.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
const char* const cPatchArchiveNames[] = {
    "FieryRoll1",
    "FieryRollsPartA",
    "FieryRollsPartABossLod",
    "GigaBell",
    "RaidonSurfFur",
    "BalanceTruckFireStep",
    "LargeHubStepABossLod",
    "NekoDisaster",
    "NekoParent",
    "NekoParentDisaster",
    "PlessieRampJumboStepBossLod",
    "PlessieRampJumboStep",
    "PlessieTunnelWallsStep",
    "DarkBowser",
    "DoorLock",
    "BlackSun",
    "KoopaJrFur",
    "CollectItemEmpty",
    "OblongSkateWheelBBossLod",
    "OblongSkateWheelLongABossLod",
    "OblongSkateWheelLongBBossLod",
    "OblongSkateStepCBossLod",
    "OblongSkateWheelABossLod",
    "BunbunFur",
    "BunbunChameleonFur",
    "TimerGate",
    "KameckFur",
    "MarioClimbGigaTail",
};
}  // namespace

static sead::DirectoryEntry sEntries[0x1000];

namespace al {
/**
 * Creates a resource by loading the archive at the given path.
 * @param rPath path of the archive
 */
Resource::Resource(const sead::SafeString& rPath)
    : mArchive(nullptr), mDevice(nullptr), mResName(rPath),
      mHeap(sead::HeapMgr::instance()->getCurrentHeap()), _B0(0), mPatchRes(nullptr), _C0(0) {
    mArchive = loadArchive(rPath);
    mDevice = new sead::ArchiveFileDevice(mArchive);
}

/**
 * Creates a resource from an already loaded archive.
 * @param rPath path of the archive
 * @param pArchive loaded archive
 */
Resource::Resource(const sead::SafeString& rPath, sead::ArchiveRes* pArchive)
    : mArchive(nullptr), mDevice(nullptr), mResName(rPath),
      mHeap(sead::HeapMgr::instance()->getCurrentHeap()), _B0(0), mPatchRes(nullptr), _C0(0) {
    mArchive = pArchive;
    mDevice = new sead::ArchiveFileDevice(mArchive);
}

/**
 * Loads the "_p" patch archive if this archive is in the patch list.
 * @return whether a patch archive was loaded
 */
bool Resource::loadPatchData() {
    for (u64 i = 0; i < sizeof(cPatchArchiveNames) / sizeof(cPatchArchiveNames[0]); i++) {
        if (isEqualString(cPatchArchiveNames[i], mResName.cstr() + 0xb)) {
            StringTmp<256> patchPath("%s_p", mResName.cstr());
            mPatchRes = new Resource(patchPath);
            return true;
        }
    }

    return false;
}

/**
 * Checks whether a file exists in this archive or its patch archives.
 * @param rFilePath path of the file
 * @return whether the file exists
 */
bool Resource::isExistFile(const sead::SafeString& rFilePath) const {
    bool isExist = false;
    mDevice->tryIsExistFile(&isExist, rFilePath);

    if (isExist) {
        return true;
    }

    if (mPatchRes != nullptr) {
        isExist = mPatchRes->isExistFile(rFilePath);
    }

    return isExist;
}

/**
 * Checks whether a byml file exists.
 * @param pFilePath path of the file without extension
 * @return whether the file exists
 */
bool Resource::isExistByml(const char* pFilePath) const {
    StringTmp<0x80> filePathExt;
    filePathExt.copy(pFilePath);
    filePathExt.append(".byml");
    return isExistFile(filePathExt.cstr());
}

/**
 * Gets the raw size of the archive.
 * @return archive size
 */
u32 Resource::getSize() const {
    return mArchive->getRawSize();
}

/**
 * Gets the number of entries in a directory.
 * @param rDirectoryPath path of the directory
 * @return number of entries
 */
u32 Resource::getEntryNum(const sead::SafeString& rDirectoryPath) const {
    sead::DirectoryHandle handle;

    if (mDevice->tryOpenDirectory(&handle, rDirectoryPath) == nullptr) {
        return 0;
    }

    u32 entryNum = mDevice->readDirectory(&handle, sEntries, 0x1000);
    mDevice->tryCloseDirectory(&handle);
    return entryNum;
}

/**
 * Gets the name of an entry in a directory.
 * @param pOutName output name
 * @param rDirectoryPath path of the directory
 * @param entryIndex index of the entry
 */
void Resource::getEntryName(sead::BufferedSafeString* pOutName,
                            const sead::SafeString& rDirectoryPath, u32 entryIndex) const {
    sead::DirectoryHandle handle;
    mDevice->tryOpenDirectory(&handle, rDirectoryPath);
    mDevice->readDirectory(&handle, sEntries, 0x1000);
    mDevice->tryCloseDirectory(&handle);
    pOutName->format("%s", sEntries[entryIndex].name.cstr());
}

/**
 * Gets the size of a file.
 * @param rFilePath path of the file
 * @return file size
 */
u32 Resource::getFileSize(const sead::SafeString& rFilePath) const {
    return mDevice->getFileSize(rFilePath);
}

/**
 * Gets a byml file.
 * @param rFilePath path of the file without extension
 * @return file data
 */
const u8* Resource::getByml(const sead::SafeString& rFilePath) const {
    StringTmp<0x80> filePathExt;
    filePathExt.copy(rFilePath);
    filePathExt.append(".byml");
    return static_cast<const u8*>(getFile(filePathExt));
}

/**
 * Gets a file, preferring the patch archives that contain it.
 * @param rFilePath path of the file
 * @return file data
 */
const void* Resource::getFile(const sead::SafeString& rFilePath) const {
    const Resource* resource = this;

    while (resource->mPatchRes != nullptr && resource->mPatchRes->isExistFile(rFilePath)) {
        resource = resource->mPatchRes;
    }

    return resource->mArchive->getFile(rFilePath);
}

/**
 * Gets a byml file if it exists.
 * @param rFilePath path of the file without extension
 * @return file data or nullptr
 */
const u8* Resource::tryGetByml(const sead::SafeString& rFilePath) const {
    StringTmp<0x80> filePathExt;
    filePathExt.copy(rFilePath);
    filePathExt.append(".byml");

    if (!isExistFile(filePathExt.cstr())) {
        return nullptr;
    }

    return static_cast<const u8*>(getFile(filePathExt));
}

/**
 * Gets a kcl file.
 * @param rFilePath path of the file without extension
 * @return file data
 */
const void* Resource::getKcl(const sead::SafeString& rFilePath) const {
    StringTmp<0x80> filePathExt;
    filePathExt.copy(rFilePath);
    filePathExt.append(".kcl");
    return getFile(filePathExt);
}

/**
 * Gets a kcl file if it exists.
 * @param rFilePath path of the file without extension
 * @return file data or nullptr
 */
const void* Resource::tryGetKcl(const sead::SafeString& rFilePath) const {
    StringTmp<0x80> filePathExt;
    filePathExt.copy(rFilePath);
    filePathExt.append(".kcl");

    if (!isExistFile(filePathExt.cstr())) {
        return nullptr;
    }

    return getFile(filePathExt);
}

/**
 * Gets a pa file.
 * @param rFilePath path of the file without extension
 * @return file data
 */
const void* Resource::getPa(const sead::SafeString& rFilePath) const {
    StringTmp<0x80> filePathExt("%s.pa", rFilePath.cstr());
    return getFile(filePathExt);
}

/**
 * Gets a file and optionally its size.
 * @param rFilePath path of the file
 * @param pSize output size, may be nullptr
 * @return file data
 */
void* Resource::getOtherFile(const sead::SafeString& rFilePath, u32* pSize) const {
    if (pSize) {
        *pSize = getFileSize(rFilePath);
    }

    return const_cast<void*>(getFile(rFilePath));
}

/**
 * Gets the archive name without directories.
 * @return archive name
 */
const char* Resource::getArchiveName() const {
    return getBaseName(mResName.cstr());
}

/**
 * Sets up the graphics file of this resource.
 * @param rFilePath path of the bfres file
 * @param pTextureFile file whose textures are bound first, may be nullptr
 * @return whether the file was set up
 */
bool Resource::tryCreateResGraphicsFile(const sead::SafeString& rFilePath,
                                        nn::g3d::ResFile* pTextureFile) {
    if (mResFile != nullptr) {
        return false;
    }

    mResFile = nn::g3d::ResFile::ResCast(const_cast<void*>(getFile(rFilePath)));

    if (pTextureFile != nullptr) {
        agl::g3d::ResFile::BindTexture(mResFile, pTextureFile);
    }

    agl::g3d::ResFile::Setup(mResFile);
    agl::g3d::ResFile::BindTexture(mResFile, mResFile);
    return true;
}

/**
 * Cleans up the graphics files of this resource and its patch archives.
 */
void Resource::cleanupResGraphicsFile() {
    if (mResFile != nullptr) {
        mResFile->ReleaseTexture();
        mResFile->Reset();
        agl::g3d::ResFile::Cleanup(mResFile);
        mResFile = nullptr;
    }

    if (mPatchRes != nullptr) {
        mPatchRes->cleanupResGraphicsFile();
    }
}
}  // namespace al
