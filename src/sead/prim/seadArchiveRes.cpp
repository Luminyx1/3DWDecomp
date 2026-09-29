#include <filedevice/seadArchiveFileDevice.h>
#include <resource/seadArchiveRes.h>
#include <resource/seadResourceMgr.h>

namespace sead
{
/**
 * Prepares the archive from the loaded data and records whether it is usable.
 * @param pBuf the archive data
 */
void ArchiveRes::doCreate_(u8* pBuf, u32, Heap*)
{
    mEnable = prepareArchive_(pBuf);
}

/**
 * Loads a resource from inside this archive through a temporary archive file device.
 * @param rArg the load settings; its device is set for the duration of the load
 * @return the loaded resource, or nullptr on failure
 */
Resource* ArchiveRes::load(ResourceMgr::LoadArg& rArg)
{
    ArchiveFileDevice device(this);
    rArg.device = &device;
    Resource* pResource = ResourceMgr::instance()->tryLoadWithoutDecomp(rArg);
    rArg.device = nullptr;
    return pResource;
}

#if SEAD_ARCHIVERES_ISEXISTFILEIMPL
/**
 * Checks whether a file exists in the archive.
 * @param rPath the file's path inside the archive
 * @return whether the file exists
 */
bool ArchiveRes::isExistFileImpl_(const SafeString& rPath) SEAD_ARCHIVERES_CONST_TOKEN
{
    return convertPathToEntryIDImpl_(rPath) != -1;
}
#endif
}  // namespace sead
