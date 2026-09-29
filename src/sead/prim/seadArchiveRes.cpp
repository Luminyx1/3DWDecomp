#include <resource/seadArchiveRes.h>

namespace sead
{
void ArchiveRes::doCreate_(u8* pBuf, u32, Heap*)
{
    mEnable = prepareArchive_(pBuf);
}

#if SEAD_ARCHIVERES_ISEXISTFILEIMPL
bool ArchiveRes::isExistFileImpl_(const SafeString& rPath) SEAD_ARCHIVERES_CONST_TOKEN
{
    return convertPathToEntryIDImpl_(rPath) != -1;
}
#endif
}  // namespace sead
