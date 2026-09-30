#include <eui/euiSharcArchive.h>

#include <resource/seadResource.h>
#include <resource/seadResourceMgr.h>

namespace eui {

/** @brief Creates an archive wrapper without a loaded resource. */
SharcArchive::SharcArchive() : m_pArchive(nullptr) {}

/** @brief Releases the loaded archive resource. */
SharcArchive::~SharcArchive() {
    finalize();
}

/** @brief Unloads the archive resource and clears its association. */
void SharcArchive::finalize() {
    if (m_pArchive) {
        sead::ResourceMgr::instance()->unload(m_pArchive);
        m_pArchive = nullptr;
    }
}

/**
 * @brief Creates an archive resource over an existing memory buffer.
 * @param[in] pHeap Heap used to allocate the archive resource.
 * @param[in] pData Archive data retained by the resource without taking ownership.
 * @param[in] size Size of the archive data buffer in bytes.
 */
void SharcArchive::initialize(sead::Heap* pHeap, void* pData, u32 size) {
    sead::ResourceMgr::CreateArg arg;
    sead::DirectResourceFactory<sead::SharcArchiveRes> factory;
    arg.buffer = static_cast<u8*>(pData);
    arg.file_size = size;
    arg.buffer_size = size;
    arg.heap = pHeap;
    arg.factory = &factory;
    m_pArchive = sead::DynamicCast<sead::SharcArchiveRes>(sead::ResourceMgr::instance()->create(arg));
}

/**
 * @brief Opens a reader at the archive's root directory.
 * @param[in,out] pReader Reader to associate with this archive and reset before iteration.
 * @return Device that opened the directory, or null if opening failed.
 */
sead::FileDevice* SharcArchive::startFileReader(FileReader* pReader) const {
    pReader->mIndex = -1;
    pReader->m_FileDevice.setArchive(m_pArchive);
    return pReader->m_FileDevice.tryOpenDirectory(&pReader->m_Handle, "");
}

/** @brief Closes the directory associated with an initialized archive reader. */
SharcArchive::FileReader::~FileReader() {
    if (m_FileDevice.getArchive()) {
        m_FileDevice.tryCloseDirectory(&m_Handle);
    }
}

/**
 * @brief Reads the next directory entry and advances the entry index.
 * @return True if one entry was read; false at the end or on a read failure.
 */
bool SharcArchive::FileReader::readNext() {
    u32 count = 0;
    m_FileDevice.tryReadDirectory(&count, &m_Handle, &m_Entry, 1);

    if (count == 1) {
        ++mIndex;
        return true;
    }

    return false;
}

}  // namespace eui
