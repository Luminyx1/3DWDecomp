#include "Library/File/FileUtil.hpp"

#include <filedevice/seadPath.h>

#include "Library/Memory/HeapUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/System/SystemKit.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/File/FileLoader.hpp"

namespace al {
const char* getLanguage();

inline FileLoader* getFileLoader() {
    return alProjectInterface::getSystemKit()->getFileLoader();
}

/**
 * Checks whether a file exists.
 * @param rFileName File path.
 * @return True if it exists.
 */
bool isExistFile(const sead::SafeString& rFileName) {
    return getFileLoader()->isExistFile(rFileName, nullptr);
}

/**
 * Checks whether a .szs archive exists.
 * @param rFileName Archive path without extension.
 * @return True if it exists.
 */
bool isExistArchive(const sead::SafeString& rFileName) {
    return getFileLoader()->isExistArchive(StringTmp<256>("%s.szs", rFileName.cstr()), nullptr);
}

/**
 * Checks whether an archive with the given extension exists.
 * @param rFileName Archive path without extension.
 * @param pExt Extension.
 * @return True if it exists.
 */
bool isExistArchive(const sead::SafeString& rFileName, const char* pExt) {
    return getFileLoader()->isExistArchive(StringTmp<256>("%s.%s", rFileName.cstr(), pExt),
                                           nullptr);
}

/**
 * Gets the size of a file.
 * @param rFileName File path.
 * @return The size.
 */
u32 getFileSize(const sead::SafeString& rFileName) {
    return getFileLoader()->getFileSize(rFileName, nullptr);
}

/**
 * Calculates the load alignment of a file from its extension.
 * @param rFileName File path.
 * @return 0x1000 for sarc and aras files, 0 otherwise.
 */
u32 calcFileAlignment(const sead::SafeString& rFileName) {
    StringTmp<32> ext("");
    sead::Path::getExt(&ext, rFileName);
    return isEqualString("sarc", ext.cstr()) || isEqualString("aras", ext.cstr()) ? 0x1000 : 0;
}

/**
 * Calculates the buffer size alignment of a file from its extension.
 * @param rFileName File path.
 * @return 0x1000 for aras files, 0 otherwise.
 */
u32 calcBufferSizeAlignment(const sead::SafeString& rFileName) {
    StringTmp<32> ext("");
    sead::Path::getExt(&ext, rFileName);
    return isEqualString("aras", ext.cstr()) ? 0x1000 : 0;
}

/**
 * Loads a file.
 * @param rFileName File path.
 * @param alignment Buffer alignment.
 * @return The loaded data.
 */
u8* loadFile(const sead::SafeString& rFileName, s32 alignment) {
    return getFileLoader()->loadFile(rFileName, alignment, nullptr);
}

/**
 * Loads a .szs archive.
 * @param rFileName Archive path without extension.
 * @return The archive.
 */
sead::ArchiveRes* loadArchive(const sead::SafeString& rFileName) {
    return getFileLoader()->loadArchive(StringTmp<256>("%s.szs", rFileName.cstr()), nullptr);
}

/**
 * Loads an archive with the given extension.
 * @param rFileName Archive path without extension.
 * @param pExt Extension.
 * @return The archive.
 */
sead::ArchiveRes* loadArchiveWithExt(const sead::SafeString& rFileName, const char* pExt) {
    return getFileLoader()->loadArchive(StringTmp<256>("%s.%s", rFileName.cstr(), pExt), nullptr);
}

/**
 * Requests an asynchronous .szs archive load.
 * @param rFileName Archive path without extension.
 * @param pHeap Heap to load into.
 * @return True if a new request was made.
 */
bool tryRequestLoadArchive(const sead::SafeString& rFileName, sead::Heap* pHeap) {
    return getFileLoader()->tryRequestLoadArchive(StringTmp<256>("%s.szs", rFileName.cstr()),
                                                  pHeap, nullptr);
}

/**
 * Loads a sound item.
 * @param itemId Sound item id.
 * @param unk Unknown.
 * @param pLoader Audio resource loader.
 */
void loadSoundItem(u32 itemId, u32 unk, IAudioResourceLoader* pLoader) {
    getFileLoader()->loadSoundItem(itemId, unk, pLoader);
}

/**
 * Stubbed out sound item request.
 * @param itemId Sound item id.
 * @return Always false.
 */
bool tryRequestLoadSoundItem(u32 itemId) {
    return false;
}

/**
 * Requests the preload list PreLoadFileList<id> of a resource.
 * @param pResource Resource holding the list.
 * @param id List number.
 * @param pHeap Heap to load into, or nullptr for the scene resource heap.
 * @param pLoader Audio resource loader.
 * @return True if the list exists.
 */
bool tryRequestPreLoadFile(const Resource* pResource, s32 id, sead::Heap* pHeap,
                           IAudioResourceLoader* pLoader) {
    return tryRequestPreLoadFile(pResource, StringTmp<256>("PreLoadFileList%d", id), pHeap,
                                 pLoader);
}

/**
 * Requests a preload list of a resource.
 * @param pResource Resource holding the list.
 * @param rFileName List file name without extension.
 * @param pHeap Heap to load into, or nullptr for the scene resource heap.
 * @param pLoader Audio resource loader.
 * @return True if the list exists.
 */
bool tryRequestPreLoadFile(const Resource* pResource, const sead::SafeString& rFileName,
                           sead::Heap* pHeap, IAudioResourceLoader* pLoader) {
    if (!pResource->isExistFile(StringTmp<256>("%s.byml", rFileName.cstr()))) {
        return false;
    }

    ByamlIter byml(pResource->getByml(rFileName));
    FileLoader* fileLoader = getFileLoader();

    if (pHeap == nullptr) {
        pHeap = getSceneResourceHeap();
    }

    fileLoader->requestPreLoadFile(byml, pHeap, pLoader);
    return true;
}

/**
 * Waits until every requested archive is loaded.
 */
void waitLoadDoneAllFile() {
    getFileLoader()->waitLoadDoneAllFile();
}

/**
 * Clears all file loader entries.
 */
void clearFileLoaderEntry() {
    getFileLoader()->clearAllEntry();
}

/**
 * Makes the path of a localized archive for the current language.
 * @param pOutPath Output path.
 * @param rFileName Archive name.
 */
void makeLocalizedArchivePath(sead::BufferedSafeString* pOutPath,
                              const sead::SafeString& rFileName) {
    pOutPath->format("LocalizedData/%s/%s", getLanguage(), rFileName.cstr());
}

/**
 * Makes the path of a localized archive for EuEnglish.
 * @param pOutPath Output path.
 * @param rFileName Archive name.
 */
void makeLocalizedArchivePathByCountryCode(sead::BufferedSafeString* pOutPath,
                                           const sead::SafeString& rFileName) {
    pOutPath->format("LocalizedData/%s/%s", "EuEnglish", rFileName.cstr());
}

/**
 * Sets the file loader thread priority.
 * @param priority Thread priority.
 */
void setFileLoaderThreadPriority(s32 priority) {
    getFileLoader()->setThreadPriority(priority);
}
}  // namespace al
