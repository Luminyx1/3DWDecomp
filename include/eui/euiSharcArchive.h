#pragma once

#include <filedevice/seadArchiveFileDevice.h>
#include <resource/seadSharcArchiveRes.h>

namespace eui {

class SharcArchive {
public:
    class FileReader {
    public:
        /** @brief Creates a reader with no associated archive or current entry. */
        FileReader() : m_FileDevice(nullptr), mIndex(-1) {}
        ~FileReader();
        bool readNext();

    private:
        friend class SharcArchive;
        sead::ArchiveFileDevice m_FileDevice;
        s32 mIndex;
        sead::DirectoryHandle m_Handle;
        sead::DirectoryEntry m_Entry;
    };

    SharcArchive();
    ~SharcArchive();
    void initialize(sead::Heap* pHeap, void* pData, u32 size);
    void finalize();
    sead::FileDevice* startFileReader(FileReader* pReader) const;

private:
    sead::SharcArchiveRes* m_pArchive;
};

}  // namespace eui
